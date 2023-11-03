/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RDC_CALL_COMPONENT_HPP__
#define __ASIO2_RDC_CALL_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <queue>
#include <any>
#include <future>
#include <tuple>
#include <type_traits>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>

#include <asio2/base/impl/condition_event_cp.hpp>

#include <asio2/component/rdc/rdc_invoker.hpp>
#include <asio2/component/rdc/rdc_option.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;

	template<class derived_t, class args_t>
	class rdc_call_cp_impl
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;

	public:
		using send_data_t = typename args_t::send_data_t;
		using recv_data_t = typename args_t::recv_data_t;
		using callback_type = std::function<void(const error_code&, send_data_t, recv_data_t)>;

		/**
		 * @brief constructor
		 */
		 rdc_call_cp_impl() noexcept = default;

		/**
		 * @brief destructor
		 */
		~rdc_call_cp_impl() noexcept = default;

	protected:
		template<class derive_t, class arg_t>
		struct sync_call_op
		{
			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

			template<class return_t>
			inline static void convert_recv_data_to_result(std::shared_ptr<return_t>& result, recv_data_t data)
			{
				if constexpr (std::is_reference_v<recv_data_t> &&
					has_equal_operator<return_t, std::remove_reference_t<recv_data_t>>::value)
				{
					*result = std::move(data);
				}
				else
				{
					if constexpr (has_stream_operator<return_t, recv_data_t>::value)
					{
						*result << data;
					}
					else if constexpr (has_equal_operator<return_t, recv_data_t>::value)
					{
						*result = data;
					}
					else
					{
						::new (result.get()) return_t(data);
					}
				}
			}

			template<class return_t, class Rep, class Period, class DataT>
			inline static return_t exec(derive_t& derive, std::chrono::duration<Rep, Period> timeout, DataT&& data)
			{
				static_assert(!std::is_void_v<return_t>);
				static_assert(!std::is_reference_v<return_t> && !std::is_pointer_v<return_t>);
				static_assert(!is_template_instance_of_v<std::basic_string_view, return_t>);

				// should't read the ecs in other thread which is not the io_context thread.
				//if (!derive.ecs_->get_rdc())
				//{
				//	ASIO2_ASSERT(false && "This function is only available in rdc mode");
				//}

				if (!derive.is_started())
				{
					set_last_error(asio::error::not_connected);

					// [20210818] don't throw an error, you can use get_last_error() to check
					// is there any exception.

					return return_t{};
				}

				std::shared_ptr<return_t> result = std::make_shared<return_t>();

				std::shared_ptr<std::promise<error_code>> promise = std::make_shared<std::promise<error_code>>();
				std::future<error_code> future = promise->get_future();

				auto invoker = [&derive, result, pm = std::move(promise)]
				(const error_code& ec, send_data_t s, recv_data_t r) mutable
				{
					ASIO2_ASSERT(derive.io_->running_in_this_thread());

					detail::ignore_unused(derive, s);

					if (!ec)
					{
						convert_recv_data_to_result(result, r);
					}

					pm->set_value(ec);
				};

				// All pending sending events will be cancelled after enter the callback below.
				asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
				[&derive, p = derive.selfptr(), timeout,
					invoker = std::move(invoker), data = std::forward<DataT>(data)]
				() mutable
				{
					derive._rdc_send(std::move(p), std::move(data), std::move(timeout), std::move(invoker));
				}));

				// Whether we run on the io_context thread
				if (!derive.io_->running_in_this_thread())
				{
					std::future_status status = future.wait_for(timeout);
					if (status == std::future_status::ready)
					{
						set_last_error(future.get());
					}
					else
					{
						set_last_error(asio::error::timed_out);
					}
				}
				else
				{
					// If invoke synchronization rdc call function in communication thread,
					// it will degenerates into async_call and the return value is empty.

					set_last_error(asio::error::in_progress);
				}

				return std::move(*result);
			}
		};

		template<class derive_t, class arg_t>
		struct async_call_op
		{
			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

			template<class DataT, class Rep, class Period, class Invoker>
			inline static void exec(derive_t& derive, DataT&& data,
				std::chrono::duration<Rep, Period> timeout, Invoker&& invoker)
			{
				// should't read the ecs in other thread which is not the io_context thread.
				//if (!derive.ecs_->get_rdc())
				//{
				//	ASIO2_ASSERT(false && "This function is only available in rdc mode");
				//}

				if (!derive.is_started())
				{
					set_last_error(asio::error::in_progress);

					derive.post_event([&derive, p = derive.selfptr(), timeout,
						invoker = std::forward<Invoker>(invoker), data = std::forward<DataT>(data)]
					(event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(g);
						derive._rdc_send(std::move(p), std::move(data), std::move(timeout), std::move(invoker));
					});

					return;
				}

				clear_last_error();

				// All pending sending events will be cancelled after enter the callback below.
				asio::post(derive.io_->context(), make_allocator(derive.wallocator(),
				[&derive, p = derive.selfptr(), timeout,
					invoker = std::forward<Invoker>(invoker), data = std::forward<DataT>(data)]
				() mutable
				{
					derive._rdc_send(std::move(p), std::move(data), std::move(timeout), std::move(invoker));
				}));
			}
		};

		template<class derive_t, class arg_t>
		class sync_caller
		{
			template <class, class>                       friend class rdc_call_cp_impl;

			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

		protected:
			sync_caller(derive_t& d) : derive(d), tm_(d.get_default_timeout()) {}
			sync_caller(sync_caller&& o) : derive(o.derive), tm_(std::move(o.tm_)) {}
			sync_caller(const sync_caller&) = delete;
			sync_caller& operator=(sync_caller&&) = delete;
			sync_caller& operator=(const sync_caller&) = delete;

		public:
			~sync_caller() = default;

			/**
			 * @brief set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline sync_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @brief set the timeout for remote data call component (means rdc), same as set_timeout
			 */
			template<class Rep, class Period>
			inline sync_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				return this->set_timeout(std::move(timeout));
			}

			template<class return_t, class DataT>
			inline return_t call(DataT&& data)
			{
				return sync_call_op<derive_t, arg_t>::template exec<return_t>(
					derive, tm_, std::forward<DataT>(data));
			}

		protected:
			derive_t&                                           derive;
			asio::steady_timer::duration                        tm_;
		};

		template<class derive_t, class arg_t>
		class async_caller
		{
			template <class, class>                       friend class rdc_call_cp_impl;

			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

		protected:
			async_caller(derive_t& d) : derive(d), tm_(d.get_default_timeout()) {}
			async_caller(async_caller&& o) : derive(o.derive),
				tm_(std::move(o.tm_)), cb_(std::move(o.cb_)), fn_(std::move(o.fn_)) {}
			async_caller(const async_caller&) = delete;
			async_caller& operator=(async_caller&&) = delete;
			async_caller& operator=(const async_caller&) = delete;

			using defer_fn = std::function<void(asio::steady_timer::duration, callback_type)>;

		public:
			~async_caller()
			{
				if (this->fn_)
				{
					(this->fn_)(std::move(this->tm_), std::move(this->cb_));
				}
			}

			/**
			 * @brief set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline async_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = timeout;
				return (*this);
			}

			/**
			 * @brief set the timeout for remote data call component (means rdc), same as set_timeout
			 */
			template<class Rep, class Period>
			inline async_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				return this->set_timeout(std::move(timeout));
			}

			template<class Callback>
			inline async_caller& response(Callback&& cb)
			{
				this->cb_ = rdc_make_callback_t<send_data_t, recv_data_t>::bind(std::forward<Callback>(cb));
				return (*this);
			}

			template<class DataT>
			inline async_caller& async_call(DataT&& data)
			{
				derive_t& deriv = derive;
				this->fn_ = [&deriv, data = std::forward<DataT>(data)]
				(asio::steady_timer::duration timeout, callback_type cb) mutable
				{
					async_call_op<derive_t, arg_t>::exec(deriv, std::move(data), std::move(timeout), std::move(cb));
				};

				return (*this);
			}

		protected:
			derive_t&                                                        derive;
			asio::steady_timer::duration                                     tm_;
			callback_type                                                    cb_;
			defer_fn                                                         fn_;
		};

		template<class derive_t, class arg_t>
		class base_caller
		{
			template <class, class>                       friend class rdc_call_cp_impl;

			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

		protected:
			base_caller(derive_t& d) : derive(d), tm_(d.get_default_timeout()) {}
			base_caller(base_caller&& o) : derive(o.derive), tm_(std::move(o.tm_)) {}
			base_caller& operator=(base_caller&&) = delete;
			base_caller(const base_caller&) = delete;
			base_caller& operator=(const base_caller&) = delete;

		public:
			~base_caller() = default;

			/**
			 * @brief set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline base_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @brief set the timeout for remote data call component (means rdc), same as set_timeout
			 */
			template<class Rep, class Period>
			inline base_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				return this->set_timeout(std::move(timeout));
			}

			template<class Callback>
			inline async_caller<derive_t, arg_t> response(Callback&& cb)
			{
				async_caller<derive_t, arg_t> caller{ derive };
				caller.set_timeout(std::move(this->tm_));
				caller.response(std::forward<Callback>(cb));
				return caller; // "caller" is local variable has RVO optimization, should't use std::move()
			}

			template<class return_t, class DataT>
			inline return_t call(DataT&& data)
			{
				return sync_call_op<derive_t, arg_t>::template exec<return_t>(
					derive, this->tm_, std::forward<DataT>(data));
			}

			template<class DataT>
			inline async_caller<derive_t, arg_t> async_call(DataT&& data)
			{
				async_caller<derive_t, arg_t> caller{ derive };
				caller.set_timeout(std::move(this->tm_));
				caller.async_call(std::forward<DataT>(data));
				return caller; // "caller" is local variable has RVO optimization, should't use std::move()
			}

		protected:
			derive_t&                                           derive;
			asio::steady_timer::duration                        tm_;
		};

	public:
		/**
		 * @brief Send data and synchronize waiting for a response or until the timeout period is reached
		 */
		template<class return_t, class DataT>
		inline return_t call(DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t, args_t>::template exec<return_t>(
				derive, derive.get_default_timeout(), std::forward<DataT>(data));
		}

		/**
		 * @brief Send data and synchronize waiting for a response or until the timeout period is reached
		 */
		template<class return_t, class DataT, class Rep, class Period>
		inline return_t call(DataT&& data, std::chrono::duration<Rep, Period> timeout)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t, args_t>::template exec<return_t>(
				derive, timeout, std::forward<DataT>(data));
		}

		/**
		 * @brief Send data and asynchronous waiting for a response or until the timeout period is reached
		 * Callback signature : void(DataType data)
		 */
		template<class DataT, class Callback>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(DataT&& data, Callback&& cb)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t, args_t>::exec(derive, std::forward<DataT>(data), derive.get_default_timeout(),
				rdc_make_callback_t<send_data_t, recv_data_t>::bind(std::forward<Callback>(cb)));
		}

		/**
		 * @brief Send data and asynchronous waiting for a response or until the timeout period is reached
		 * Callback signature : void(DataType data)
		 */
		template<class DataT, class Callback, class Rep, class Period>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(DataT&& data, Callback&& cb, std::chrono::duration<Rep, Period> timeout)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t, args_t>::exec(derive, std::forward<DataT>(data), timeout,
				rdc_make_callback_t<send_data_t, recv_data_t>::bind(std::forward<Callback>(cb)));
		}

		/**
		 * @brief Send data and asynchronous waiting for a response or until the timeout period is reached
		 */
		template<class DataT>
		inline async_caller<derived_t, args_t> async_call(DataT&& data)
		{
			async_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.async_call(std::forward<DataT>(data));
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @brief set the timeout for remote data call component (means rdc)
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t, args_t> set_timeout(std::chrono::duration<Rep, Period> timeout)
		{
			base_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.set_timeout(timeout);
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @brief set the timeout for remote data call component (means rdc), same as set_timeout
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t, args_t> timeout(std::chrono::duration<Rep, Period> timeout)
		{
			return this->set_timeout(std::move(timeout));
		}

		/**
		 * @brief bind a response callback function for remote data call component (means rdc)
		 */
		template<class Callback>
		inline async_caller<derived_t, args_t> response(Callback&& cb)
		{
			async_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.response(std::forward<Callback>(cb));
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}
	
	protected:
		template<typename C>
		inline void _rdc_init(std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);
		}

		template<typename C>
		inline void _rdc_start(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(this_ptr, ecs);

			if constexpr (ecs_helper::has_rdc<C>())
			{
				ASIO2_ASSERT(ecs->get_component().rdc_option(std::in_place)->invoker().reqs().empty());
			}
			else
			{
				std::ignore = true;
			}
		}

		inline void _rdc_stop()
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			// this callback maybe executed after the session stop, and the session
			// stop will destroy the ecs, so we need check whether the ecs is valid.
			if (!derive.ecs_)
				return;

			asio2::rdc::option_base* prdc = derive.ecs_->get_rdc();

			// maybe not rdc mode, so must check whether the rdc is valid.
			if (!prdc)
				return;

			// if stoped, execute all callbacks in the requests, otherwise if some
			// call or async_call 's timeout is too long, then the application will
			// can't exit before all timeout timer is reached.
			prdc->foreach_and_clear([&derive](void*, void* val) mutable
			{
				std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>& tp =
					*((std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>*)val);

				auto& [timer, invoker] = tp;

				detail::cancel_timer(*timer);

				derive._rdc_invoke_with_none(asio::error::operation_aborted, invoker);
			});
		}

		template<class DataT>
		void _rdc_send(
			std::shared_ptr<derived_t> this_ptr, DataT&& data,
			std::chrono::steady_clock::duration timeout, callback_type invoker)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			auto persisted_data = derive._data_persistence(std::forward<DataT>(data));

			send_data_t sent_typed_data = derive._rdc_convert_to_send_data(persisted_data);

			if (!derive.ecs_)
			{
				derive._rdc_invoke_with_send(asio::error::operation_aborted, invoker, sent_typed_data);
				return;
			}

			// if ecs is valid, the rdc must be valid.
			asio2::rdc::option_base* prdc = derive.ecs_->get_rdc();

			// maybe not rdc mode, so must check whether the rdc is valid.
			if (!prdc)
			{
				ASIO2_ASSERT(false && "This function is only available in rdc mode");
				derive._rdc_invoke_with_send(asio::error::operation_not_supported, invoker, sent_typed_data);
				return;
			}

			std::any rdc_id = prdc->call_parser(true, (void*)std::addressof(sent_typed_data));

			std::shared_ptr<asio::steady_timer> timer =
				std::make_shared<asio::steady_timer>(derive.io_->context());

			std::tuple tp(timer, std::move(invoker));

			prdc->emplace_request(rdc_id, (void*)std::addressof(tp));

			auto ex = [&derive, rdc_id = std::move(rdc_id), prdc]() mutable
			{
				ASIO2_ASSERT(derive.io_->running_in_this_thread());

				prdc->execute_and_erase(rdc_id, [&derive](void* val) mutable
				{
					std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>& tp =
						*((std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>*)val);

					auto& [tmer, cb] = tp;

					detail::cancel_timer(*tmer);

					derive._rdc_invoke_with_none(asio::error::timed_out, cb);
				});
			};

			timer->expires_after(timeout);
			timer->async_wait([this_ptr, ex](const error_code& ec) mutable
			{
				if (ec == asio::error::operation_aborted)
					return;

				ex();
			});

			derive.push_event(
			[&derive, p = std::move(this_ptr),
				life_id = derive.life_id(), ex = std::move(ex), data = std::move(persisted_data)]
			(event_queue_guard<derived_t> g) mutable
			{
				if (life_id != derive.life_id())
				{
					set_last_error(asio::error::operation_aborted);
					ex();
					return;
				}

				derive._do_send(data, [&ex, g = std::move(g)](const error_code& ec, std::size_t) mutable
				{
					detail::ignore_unused(g);

					if (ec)
					{
						ex();
					}
				});
			});
		}

		template<typename C>
		inline void _do_rdc_handle_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, recv_data_t data)
		{
			detail::ignore_unused(this_ptr);

			derived_t& derive = static_cast<derived_t&>(*this);

			auto _rdc = ecs->get_component().rdc_option(std::in_place);

			if (_rdc->invoker().reqs().empty())
				return;

			auto rdc_id = (_rdc->get_recv_parser())(data);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			auto iter = _rdc->invoker().find(rdc_id);
			if (iter != _rdc->invoker().end())
			{
				auto&[timer, invoker] = iter->second;

				detail::cancel_timer(*timer);

				derive._rdc_invoke_with_recv(error_code{}, invoker, data);

				_rdc->invoker().erase(iter);
			}
		}

		template<typename C>
		inline void _rdc_handle_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, recv_data_t data)
		{
			if constexpr (ecs_helper::has_rdc<C>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				derive._do_rdc_handle_recv(this_ptr, ecs, data);
			}
			else
			{
				detail::ignore_unused(this_ptr, ecs, data);
			}
		}
	};

	template<class args_t>
	struct is_rdc_call_cp_enabled
	{
		template<class, class = void>
		struct has_member_enabled : std::false_type {};

		template<class T>
		struct has_member_enabled<T, std::void_t<decltype(T::rdc_call_cp_enabled)>> : std::true_type {};

		static constexpr bool value()
		{
			if constexpr (has_member_enabled<args_t>::value)
			{
				return args_t::rdc_call_cp_enabled;
			}
			else
			{
				return true;
			}
		}
	};

	template<class derived_t, class args_t, bool Enable = is_rdc_call_cp_enabled<args_t>::value()>
	class rdc_call_cp_bridge;

	template<class derived_t, class args_t>
	class rdc_call_cp_bridge<derived_t, args_t, true> : public rdc_call_cp_impl<derived_t, args_t> {};

	template<class derived_t, class args_t>
	class rdc_call_cp_bridge<derived_t, args_t, false>
	{
	protected:
		// just placeholders
		template<typename... Args> inline void _rdc_init       (Args const& ...) {}
		template<typename... Args> inline void _rdc_start      (Args const& ...) {}
		template<typename... Args> inline void _rdc_stop       (Args const& ...) {}
		template<typename... Args> inline void _rdc_handle_recv(Args const& ...) {}
	};

	template<class derived_t, class args_t>
	class rdc_call_cp : public rdc_call_cp_bridge<derived_t, args_t> {};
}

#endif // !__ASIO2_RDC_CALL_COMPONENT_HPP__
