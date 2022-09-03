/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/component/condition_event_cp.hpp>

#include <asio2/ecs/rdc/rdc_invoker.hpp>
#include <asio2/ecs/rdc/rdc_option.hpp>

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

		/**
		 * @constructor
		 */
		rdc_call_cp_impl() {}

		/**
		 * @destructor
		 */
		~rdc_call_cp_impl() = default;

	protected:
		struct rdc_fields
		{
			/// Used to notify the send queue to begin send.
			std::shared_ptr<condition_event>     send_event_;

			/// The pending send datas
			std::queue<std::tuple<
				std::function<send_data_t()>,
				std::chrono::steady_clock::duration,
				std::function<void(const error_code&, send_data_t, recv_data_t)>
				>>                               pending_datas_;

			/// Use this to ensure all rdc calls is completed, we can exit.
			bool                                 stop_flag_ = false;

			/// Constructor
			rdc_fields(io_t& io) : send_event_(std::make_shared<condition_event>(io))
			{
			}
		};

	protected:
		template<class derive_t, class arg_t>
		struct sync_call_op
		{
			using send_data_t = typename arg_t::send_data_t;
			using recv_data_t = typename arg_t::recv_data_t;

			template<class return_t>
			inline static void convert_recv_data_to_result(std::shared_ptr<return_t>& result, recv_data_t recv_data)
			{
				if constexpr (std::is_reference_v<recv_data_t> &&
					has_equal_operator<return_t, std::remove_reference_t<recv_data_t>>::value)
				{
					*result = std::move(recv_data);
				}
				else
				{
					if constexpr (has_stream_operator<return_t, recv_data_t>::value)
					{
						*result << recv_data;
					}
					else if constexpr (has_equal_operator<return_t, recv_data_t>::value)
					{
						*result = recv_data;
					}
					else
					{
						::new (result.get()) return_t(recv_data);
					}
				}
			}

			template<class return_t, class Rep, class Period, class DataT>
			inline static return_t exec(derive_t& derive, std::chrono::duration<Rep, Period> timeout, DataT&& data)
			{
				static_assert(!std::is_void_v<return_t>);
				static_assert(!std::is_reference_v<return_t> && !std::is_pointer_v<return_t>);
				static_assert(!is_template_instance_of_v<std::basic_string_view, return_t>);

				ASIO2_ASSERT(derive.rdc_fields_ && "This function is only available in rdc mode");

				decltype(auto) fn_data = [&derive, data = derive._data_persistence(std::forward<DataT>(data))]
				() mutable->send_data_t
				{
					return derive._rdc_convert_to_send_data(data);
				};

				error_code ec;
				std::shared_ptr<return_t> result = std::make_shared<return_t>();
				try
				{
					if (!derive.rdc_fields_)
						asio::detail::throw_error(asio::error::operation_not_supported);

					if (!derive.is_started())
						asio::detail::throw_error(asio::error::not_connected);

					std::shared_ptr<std::promise<error_code>> promise =
						std::make_shared<std::promise<error_code>>();
					std::future<error_code> future = promise->get_future();

					auto invoker = [&derive, result, pm = std::move(promise)]
					(const error_code& ec, send_data_t s, recv_data_t r) mutable
					{
						ASIO2_ASSERT(derive.io().running_in_this_thread());

						detail::ignore_unused(derive, s);

						if (!ec)
						{
							convert_recv_data_to_result(result, r);
						}

						pm->set_value(ec);
					};

					// All pending sending events will be cancelled after enter the callback below.
					derive.push_event(
					[&derive, p = derive.selfptr(), timeout, invoker = std::move(invoker), fn_data = std::move(fn_data)]
					(event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(p, g);

						derive.rdc_fields_->pending_datas_.emplace(
							std::move(fn_data), std::move(timeout), std::move(invoker));

						derive.rdc_fields_->send_event_->notify();
					});

					// Whether we run on the io_context thread
					if (!derive.io().running_in_this_thread())
					{
						std::future_status status = future.wait_for(timeout);
						if (status == std::future_status::ready)
						{
							ec = future.get();
						}
						else
						{
							ec = asio::error::timed_out;
						}
					}
					else
					{
						// If invoke synchronization rdc call function in communication thread, it will degenerates
						// into async_call and the return value is empty.

						ec = asio::error::in_progress;
					}
				}
				catch (system_error      const& e) { ec = e.code(); }
				catch (std::exception    const&  ) { ec = asio::error::eof; }

				set_last_error(ec);

				// [20210818] don't throw an error, you can use get_last_error() to check
				// is there any exception.
				// asio::detail::throw_error(ec);

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
				error_code ec;

				ASIO2_ASSERT(derive.rdc_fields_ && "This function is only available in rdc mode");

				decltype(auto) fn_data = [&derive, data = derive._data_persistence(std::forward<DataT>(data))]
				() mutable->send_data_t
				{
					return derive._rdc_convert_to_send_data(data);
				};

				try
				{
					if (!derive.rdc_fields_)
						asio::detail::throw_error(asio::error::operation_not_supported);

					if (!derive.is_started())
						asio::detail::throw_error(asio::error::not_connected);

					// All pending sending events will be cancelled after enter the callback below.
					derive.push_event([&derive, p = derive.selfptr(), timeout,
						invoker = std::forward<Invoker>(invoker), fn_data = std::move(fn_data)]
					(event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(p, g);

						derive.rdc_fields_->pending_datas_.emplace(
							std::move(fn_data), std::move(timeout), std::move(invoker));

						derive.rdc_fields_->send_event_->notify();
					});

					return;
				}
				catch (system_error     & e) { ec = e.code();             }
				catch (std::exception   &  ) { ec = asio::error::eof;     }

				set_last_error(ec);

				// ensure the callback was called in the communication thread.
				derive.push_event([&derive, p = derive.selfptr(), ec, invoker = std::forward<Invoker>(invoker),
					fn_data = std::move(fn_data)](event_queue_guard<derived_t> g) mutable
				{
					detail::ignore_unused(p, g);

					set_last_error(ec);

					derive._rdc_invoke_with_send(ec, invoker, fn_data);
				});
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
			 * @function : set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline sync_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @function : set the timeout for remote data call component (means rdc), same as set_timeout
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

			using defer_fn = std::function<void(asio::steady_timer::duration,
				std::function<void(const error_code&, send_data_t, recv_data_t)>)>;

		public:
			~async_caller()
			{
				if (this->fn_)
				{
					(this->fn_)(std::move(this->tm_), std::move(this->cb_));
				}
			}

			/**
			 * @function : set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline async_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = timeout;
				return (*this);
			}

			/**
			 * @function : set the timeout for remote data call component (means rdc), same as set_timeout
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
				this->fn_ = [&deriv, data = std::forward<DataT>(data)](asio::steady_timer::duration timeout,
					std::function<void(const error_code&, send_data_t, recv_data_t)> cb) mutable
				{
					async_call_op<derive_t, arg_t>::exec(deriv, std::move(data), std::move(timeout), std::move(cb));
				};

				return (*this);
			}

		protected:
			derive_t&                                                        derive;
			asio::steady_timer::duration                                     tm_;
			std::function<void(const error_code&, send_data_t, recv_data_t)> cb_;
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
			 * @function : set the timeout for remote data call component (means rdc)
			 */
			template<class Rep, class Period>
			inline base_caller& set_timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @function : set the timeout for remote data call component (means rdc), same as set_timeout
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
		 * @function : Send data and synchronize waiting for a response or until the timeout period is reached
		 */
		template<class return_t, class DataT>
		inline return_t call(DataT&& data)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t, args_t>::template exec<return_t>(
				derive, derive.get_default_timeout(), std::forward<DataT>(data));
		}

		/**
		 * @function : Send data and synchronize waiting for a response or until the timeout period is reached
		 */
		template<class return_t, class DataT, class Rep, class Period>
		inline return_t call(DataT&& data, std::chrono::duration<Rep, Period> timeout)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t, args_t>::template exec<return_t>(
				derive, timeout, std::forward<DataT>(data));
		}

		/**
		 * @function : Send data and asynchronous waiting for a response or until the timeout period is reached
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
		 * @function : Send data and asynchronous waiting for a response or until the timeout period is reached
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
		 * @function : Send data and asynchronous waiting for a response or until the timeout period is reached
		 */
		template<class DataT>
		inline async_caller<derived_t, args_t> async_call(DataT&& data)
		{
			async_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.async_call(std::forward<DataT>(data));
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @function : set the timeout for remote data call component (means rdc)
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t, args_t> set_timeout(std::chrono::duration<Rep, Period> timeout)
		{
			base_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.set_timeout(timeout);
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @function : set the timeout for remote data call component (means rdc), same as set_timeout
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t, args_t> timeout(std::chrono::duration<Rep, Period> timeout)
		{
			return this->set_timeout(std::move(timeout));
		}

		/**
		 * @function : bind a response callback function for remote data call component (means rdc)
		 */
		template<class Callback>
		inline async_caller<derived_t, args_t> response(Callback&& cb)
		{
			async_caller<derived_t, args_t> caller{ static_cast<derived_t&>(*this) };
			caller.response(std::forward<Callback>(cb));
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}
	
	protected:
		template<typename MatchCondition>
		inline void _rdc_init(condition_wrap<MatchCondition>& condition)
		{
			detail::ignore_unused(condition);

			if constexpr (condition_helper::has_rdc<MatchCondition>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				this->rdc_fields_ = std::make_unique<rdc_fields>(derive.io());
			}
			else
			{
				std::ignore = true;
			}
		}

		template<typename MatchCondition>
		inline void _rdc_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if constexpr (condition_helper::has_rdc<MatchCondition>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				if (this->rdc_fields_)
				{
					this->rdc_fields_->stop_flag_ = false;
				}

				derive._rdc_post_wait(std::move(this_ptr), std::move(condition));
			}
			else
			{
				detail::ignore_unused(this_ptr, condition);
			}
		}

		inline void _rdc_stop()
		{
			if (this->rdc_fields_)
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				// All pending sending events will be cancelled after enter the callback below.
				derive.push_event([this, p = derive.selfptr()](event_queue_guard<derived_t> g) mutable
				{
					detail::ignore_unused(p, g);

					this->rdc_fields_->stop_flag_ = true;
					this->rdc_fields_->send_event_->notify();
				});
			}
		}

		template<typename MatchCondition>
		inline void _rdc_post_wait(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if constexpr (condition_helper::has_rdc<MatchCondition>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				ASIO2_ASSERT(this->rdc_fields_);

				if (!this->rdc_fields_)
					return;

				this->rdc_fields_->send_event_->async_wait(
				[&derive, this, this_ptr = std::move(this_ptr), condition = std::move(condition)]() mutable
				{
					ASIO2_ASSERT(derive.io().running_in_this_thread());

					while (!this->rdc_fields_->pending_datas_.empty())
					{
						auto&&[fn_data, timeout, invoker] = this->rdc_fields_->pending_datas_.front();

						derive._rdc_post_send(this_ptr, condition,
							std::move(fn_data), std::move(timeout), std::move(invoker));

						this->rdc_fields_->pending_datas_.pop();
					}

					if (!this->rdc_fields_->stop_flag_)
					{
						derive._rdc_post_wait(std::move(this_ptr), std::move(condition));
					}
					else
					{
						auto _rdc = condition.impl_->rdc_option(std::in_place);

						// if stoped, execute all callbacks in the requests, otherwise if some
						// call or async_call 's timeout is too long, then the application will
						// can't exit before all timeout timer is reached.
						for (auto&[id, tup] : _rdc->invoker().reqs())
						{
							detail::ignore_unused(id);
							auto&[timer, invoker] = tup;
							try
							{
								timer->cancel();
							}
							catch (system_error const&)
							{
							}
							derive._rdc_invoke_with_none(asio::error::operation_aborted, invoker);
						}
						_rdc->invoker().reqs().clear();
					}
				});
			}
			else
			{
				std::ignore = true;
			}
		}

		template<typename MatchCondition>
		inline bool _rdc_post_send(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition,
			std::function<send_data_t()>&& fn_data,
			std::chrono::steady_clock::duration&& timeout,
			std::function<void(const error_code&, send_data_t, recv_data_t)>&& invoker)
		{
			if constexpr (condition_helper::has_rdc<MatchCondition>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				try
				{
					auto _rdc = condition.impl_->rdc_option(std::in_place);

					auto id = (_rdc->get_send_parser())(fn_data());

					std::shared_ptr<asio::steady_timer> timer =
						std::make_shared<asio::steady_timer>(derive.io().context());

					derive.push_event([&derive, this_ptr = std::move(this_ptr), condition = std::move(condition),
						id = std::move(id), timer = std::move(timer),
						fn_data = std::move(fn_data), timeout, invoker = std::move(invoker)]
					(event_queue_guard<derived_t> g) mutable
					{
						auto _rdc = condition.impl_->rdc_option(std::in_place);

						ASIO2_ASSERT(derive.io().running_in_this_thread());

						_rdc->invoker().emplace(id, timer, std::move(invoker));

						timer->expires_after(timeout);
						timer->async_wait(
						[&derive, id, this_ptr = std::move(this_ptr), condition]
						(const error_code& ec) mutable
						{
							if (ec == asio::error::operation_aborted)
								return;

							auto _rdc = condition.impl_->rdc_option(std::in_place);

							ASIO2_ASSERT(derive.io().running_in_this_thread());

							auto iter = _rdc->invoker().find(id);
							if (iter != _rdc->invoker().end())
							{
								auto&[tmer, cb] = iter->second;
								derive._rdc_invoke_with_none(asio::error::timed_out, cb);
								_rdc->invoker().erase(iter);
								detail::ignore_unused(tmer);
							}
						});

						send_data_t data = fn_data();

						derive._do_send(data,
						[&derive, id = std::move(id), condition = std::move(condition), g = std::move(g)]
						(const error_code& ec, std::size_t) mutable
						{
							detail::ignore_unused(g);

							if (ec)
							{
								auto _rdc = condition.impl_->rdc_option(std::in_place);

								ASIO2_ASSERT(derive.io().running_in_this_thread());

								auto iter = _rdc->invoker().find(id);
								if (iter != _rdc->invoker().end())
								{
									auto&[tmer, cb] = iter->second;
									try
									{
										tmer->cancel();
									}
									catch (system_error const&)
									{
									}
									derive._rdc_invoke_with_none(asio::error::timed_out, cb);
									_rdc->invoker().erase(iter);
								}
							}
						});
					});
					return true;
				}
				catch (system_error & e) { set_last_error(e); }
				catch (std::exception &) { set_last_error(asio::error::eof); }

				derive._rdc_invoke_with_send(get_last_error(), invoker, fn_data);

				return false;
			}
			else
			{
				std::ignore = true;
			}
		}

		template<typename MatchCondition>
		inline void _rdc_handle_recv(std::shared_ptr<derived_t>& this_ptr, recv_data_t data,
			condition_wrap<MatchCondition>& condition)
		{
			detail::ignore_unused(this_ptr, data, condition);

			if constexpr (condition_helper::has_rdc<MatchCondition>())
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				auto _rdc = condition.impl_->rdc_option(std::in_place);

				auto id = (_rdc->get_recv_parser())(data);

				ASIO2_ASSERT(derive.io().running_in_this_thread());

				auto iter = _rdc->invoker().find(id);
				if (iter != _rdc->invoker().end())
				{
					auto&[timer, invoker] = iter->second;
					try
					{
						timer->cancel();
					}
					catch (system_error const&)
					{
					}
					derive._rdc_invoke_with_recv(error_code{}, invoker, data);
					_rdc->invoker().erase(iter);
				}
			}
			else
			{
				std::ignore = true;
			}
		}

	protected:
		std::unique_ptr<rdc_fields>              rdc_fields_;
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
