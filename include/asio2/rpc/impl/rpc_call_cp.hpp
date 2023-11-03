/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

/*
Perfect capture in C++20

template <typename ... Args>
auto f(Args&& args){
	return [... args = std::forward<Args>(args)]{
		// use args
	};
}

C++17 and C++14 workaround

In C++17 we can use a workaround with tuples:

template <typename ... Args>
auto f(Args&& ... args){
	return [args = std::make_tuple(std::forward<Args>(args) ...)]()mutable{
		return std::apply([](auto&& ... args){
			// use args
		}, std::move(args));
	};
}
*/

#ifndef __ASIO2_RPC_CALL_COMPONENT_HPP__
#define __ASIO2_RPC_CALL_COMPONENT_HPP__

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
#include <unordered_map>
#include <type_traits>
#include <map>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/function_traits.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>
#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class rpc_call_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		rpc_call_cp(rpc_serializer& sr, rpc_deserializer& dr)
			: sr_(sr), dr_(dr)
		{
		}

		/**
		 * @brief destructor
		 */
		~rpc_call_cp() = default;

	protected:
		template<class derive_t>
		struct sync_call_op
		{
			template<class return_t, class Rep, class Period, class ...Args>
			inline static return_t exec(derive_t& derive,
				std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
			{
				using result_t = typename rpc_result_t<return_t>::type;

				if (!derive.is_started())
				{
					set_last_error(rpc::make_error_code(rpc::error::not_connected));

					if constexpr (!std::is_void_v<return_t>)
					{
						return result_t{};
					}
					else
					{
						return;
					}
				}

				std::shared_ptr<result_t> result = std::make_shared<result_t>();

				set_last_error(rpc::make_error_code(rpc::error::success));

				rpc_header::id_type id = derive.mkid();
				rpc_request<Args...> req(id, std::move(name), std::forward<Args>(args)...);

				std::shared_ptr<std::promise<error_code>> promise = std::make_shared<std::promise<error_code>>();
				std::future<error_code> future = promise->get_future();

				auto ex = [&derive, result, id, pm = std::move(promise)]
				(error_code ec, std::string_view data) mutable
				{
					detail::ignore_unused(data);

					// when async_send failed, the error category is not rpc category.
					//ASIO2_ASSERT(std::string_view(ec.category().name()) == rpc::rpc_category().name());

					ASIO2_ASSERT(derive.io_->running_in_this_thread());

					if (!ec)
					{
					#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
						try
						{
					#endif
							derive.dr_ >> ec;

							if constexpr (!std::is_void_v<return_t>)
							{
								if (!ec)
									derive.dr_ >> (*result);
							}
							else
							{
								std::ignore = result;
							}
					#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
						}
						catch (cereal::exception const&)
						{
							ec = rpc::make_error_code(rpc::error::no_data);
						}
						catch (std::exception    const&)
						{
							ec = rpc::make_error_code(rpc::error::unspecified_error);
						}
					#endif
					}

					if (std::addressof(ec.category()) != std::addressof(rpc::rpc_category()))
					{
						ec.assign(ec.value(), rpc::rpc_category());
					}

					ASIO2_ASSERT(std::string_view(ec.category().name()) == rpc::rpc_category().name());

					set_last_error(ec);

					pm->set_value(ec);

					derive.reqs_.erase(id);
				};

				asio::post(derive.io_->context(), make_allocator(derive.callocator_,
				[&derive, req = std::move(req), ex = std::move(ex), p = derive.selfptr()]() mutable
				{
					derive.reqs_.emplace(req.id(), std::move(ex));

					derive.internal_async_send(std::move(p), (derive.sr_.reset() << req).str(),
					[&derive, id = req.id()]() mutable
					{
						if (get_last_error()) // send data failed with error
						{
							auto iter = derive.reqs_.find(id);
							if (iter != derive.reqs_.end())
							{
								auto& ex = iter->second;
								ex(get_last_error(), std::string_view{});
							}
						}
					});
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
						set_last_error(rpc::make_error_code(rpc::error::timed_out));

						asio::post(derive.io_->context(), make_allocator(derive.callocator_,
						[&derive, id, p = derive.selfptr()]() mutable
						{
							detail::ignore_unused(p);

							derive.reqs_.erase(id);
						}));
					}
				}
				else
				{
					// If invoke synchronization rpc call function in communication thread, it will degenerates
					// into async_call and the return value is empty.

					asio::post(derive.io_->context(), make_allocator(derive.callocator_,
					[&derive, id, p = derive.selfptr()]() mutable
					{
						detail::ignore_unused(p);

						derive.reqs_.erase(id);
					}));

					set_last_error(rpc::make_error_code(rpc::error::in_progress));
				}

				ASIO2_ASSERT(std::string_view(get_last_error().category().name()) == rpc::rpc_category().name());

				// [20210818] don't throw an error, you can use get_last_error() to check
				// is there any exception.

				if constexpr (!std::is_void_v<return_t>)
				{
					return std::move(*result);
				}
				else
				{
					return;
				}
			}
		};

		template<class derive_t>
		struct async_call_op
		{
			template<class Callback>
			inline static auto to_safe_callback(Callback&& cb)
			{
				using cbtype = typename detail::remove_cvref_t<Callback>;

				if constexpr (detail::has_bool_operator<cbtype>::value)
				{
					using fun_traits_type = function_traits<cbtype>;

					if (!cb)
						return cbtype{ typename fun_traits_type::stl_lambda_type{} };
					else
						return std::forward<Callback>(cb);
				}
				else
				{
					return std::forward<Callback>(cb);
				}
			}

			template<class return_t, class Callback>
			inline static auto make_callback(derive_t& derive, Callback&& cb)
			{
				return async_call_op<derive_t>::template make_callback_impl<return_t>(
					derive, async_call_op<derive_t>::to_safe_callback(std::forward<Callback>(cb)));
			}

			template<class Callback>
			inline static auto make_callback(derive_t& derive, Callback&& cb)
			{
				using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
				return async_call_op<derive_t>::template make_callback_argc(
					derive, async_call_op<derive_t>::to_safe_callback(std::forward<Callback>(cb)),
					std::integral_constant<int, fun_traits_type::argc>{});
			}

			template<class Callback>
			inline static auto make_callback_argc(derive_t& derive, Callback&& cb, std::integral_constant<int, 0>)
			{
				return async_call_op<derive_t>::template make_callback_impl<void>(
					derive, std::forward<Callback>(cb));
			}

			template<class Callback>
			inline static auto make_callback_argc(derive_t& derive, Callback&& cb, std::integral_constant<int, 1>)
			{
				using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
				using return_type = typename fun_traits_type::template args<0>::type;
				static_assert(!std::is_same_v<return_type, void>);
				return async_call_op<derive_t>::template make_callback_impl<return_type>(
					derive, std::forward<Callback>(cb));
			}

			template<class return_t, class Callback>
			inline static auto make_callback_impl_0(derive_t& derive, Callback&& cb)
			{
				return [&derive, cb = std::forward<Callback>(cb)](auto ec, std::string_view) mutable
				{
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					try
					{
				#endif
						if (!ec)
							derive.dr_ >> ec;
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					}
					catch (cereal::exception const&)
					{
						ec = rpc::make_error_code(rpc::error::no_data);
					}
					catch (std::exception    const&)
					{
						ec = rpc::make_error_code(rpc::error::unspecified_error);
					}
				#endif

					if (std::addressof(ec.category()) != std::addressof(rpc::rpc_category()))
					{
						ec.assign(ec.value(), rpc::rpc_category());
					}

					ASIO2_ASSERT(std::string_view(ec.category().name()) == rpc::rpc_category().name());

					set_last_error(ec);

					cb();
				};
			}

			template<class return_t, class Callback>
			inline static auto make_callback_impl_1(derive_t& derive, Callback&& cb)
			{
				return [&derive, cb = std::forward<Callback>(cb)](auto ec, std::string_view data) mutable
				{
					detail::ignore_unused(data);

					typename rpc_result_t<return_t>::type result{};

				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					try
					{
				#endif
						if (!ec)
							derive.dr_ >> ec;

						if (!ec)
							derive.dr_ >> result;
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					}
					catch (cereal::exception const&)
					{
						ec = rpc::make_error_code(rpc::error::no_data);
					}
					catch (std::exception    const&)
					{
						ec = rpc::make_error_code(rpc::error::unspecified_error);
					}
				#endif

					if (std::addressof(ec.category()) != std::addressof(rpc::rpc_category()))
					{
						ec.assign(ec.value(), rpc::rpc_category());
					}

					ASIO2_ASSERT(std::string_view(ec.category().name()) == rpc::rpc_category().name());

					set_last_error(ec);

					cb(std::move(result));
				};
			}

			template<class return_t, class Callback>
			inline static auto make_callback_impl(derive_t& derive, Callback&& cb)
			{
				if constexpr (std::is_same_v<return_t, void>)
				{
					return async_call_op<derive_t>::template make_callback_impl_0<return_t>(
						derive, std::forward<Callback>(cb));
				}
				else
				{
					return async_call_op<derive_t>::template make_callback_impl_1<return_t>(
						derive, std::forward<Callback>(cb));
				}
			}

			template<class Req>
			inline static void exec(derive_t& derive, Req&& req)
			{
				ASIO2_ASSERT(!req.id());

				if (!derive.is_started())
				{
					set_last_error(rpc::make_error_code(rpc::error::in_progress));

					derive.post_event([&derive, req = std::forward<Req>(req), p = derive.selfptr()]
					(event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(g);
						derive.internal_async_send(std::move(p), (derive.sr_.reset() << req).str());
					});

					return;
				}

				set_last_error(rpc::make_error_code(rpc::error::success));

				asio::post(derive.io_->context(), make_allocator(derive.callocator_,
				[&derive, req = std::forward<Req>(req), p = derive.selfptr()]() mutable
				{
					derive.internal_async_send(std::move(p), (derive.sr_.reset() << req).str());
				}));
			}

			template<class Callback, class Rep, class Period, class Req>
			inline static void exec(derive_t& derive, rpc_header::id_type id,
				std::chrono::duration<Rep, Period> timeout, Callback&& cb, Req&& req)
			{
				ASIO2_ASSERT(id);

				req.id(id);

				// 2020-12-03 Fix possible bug: move the "timer->async_wait" into the io_context thread.
				// otherwise the "derive.send" maybe has't called, the "timer->async_wait" has called
				// already.
				std::shared_ptr<asio::steady_timer> timer =
					std::make_shared<asio::steady_timer>(derive.io_->context());

				auto ex = [&derive, id, timer, cb = std::forward<Callback>(cb)]
				(error_code ec, std::string_view data) mutable
				{
					ASIO2_ASSERT(derive.io_->running_in_this_thread());

					detail::cancel_timer(*timer);

					cb(ec, data);

					derive.reqs_.erase(id);
				};

				auto fn = [&derive, timer = std::move(timer), timeout, req = std::forward<Req>(req),
					ex = std::move(ex), p = derive.selfptr()]() mutable
				{
					// 1. first, save the request id
					derive.reqs_.emplace(req.id(), std::move(ex));

					// 2. second, start the timeout timer.
					// note : cannot put "start timer" after "async_send", beacust the async_send
					// maybe failed immediately with the "derive.is_started() == false", then the
					// callback of async_send will be called immediately, and the "ex" will be called,
					// and the timer will be canceled, but at this time, the timer has't start yet,
					// when async_send is return, the timer will be begin "async_wait", but the timer
					// "cancel" is called already, so this will cause some small problem.

					// must start a timeout timer, othwise if not recved response, it will cause the
					// request id in the map forever.

					timer->expires_after(timeout);
					timer->async_wait(
					[p, &derive, id = req.id()]
					(const error_code& ec) mutable
					{
						detail::ignore_unused(p);

						if (ec == asio::error::operation_aborted)
							return;

						auto iter = derive.reqs_.find(id);
						if (iter != derive.reqs_.end())
						{
							auto& ex = iter->second;
							ex(rpc::make_error_code(rpc::error::timed_out), std::string_view{});
						}
					});

					// 3. third, send request.
					derive.internal_async_send(std::move(p), (derive.sr_.reset() << req).str(),
					[&derive, id = req.id()]() mutable
					{
						if (get_last_error()) // send data failed with error
						{
							auto iter = derive.reqs_.find(id);
							if (iter != derive.reqs_.end())
							{
								auto& ex = iter->second;
								ex(get_last_error(), std::string_view{});
							}
						}
					});
				};

				if (!derive.is_started())
				{
					set_last_error(rpc::make_error_code(rpc::error::in_progress));

					derive.post_event([fn = std::move(fn)](event_queue_guard<derived_t> g) mutable
					{
						detail::ignore_unused(g);
						fn();
					});

					return;
				}

				set_last_error(rpc::make_error_code(rpc::error::success));

				// 2019-11-28 fixed the bug of issue #6 : task() cannot be called directly

				// 2021-12-10 : can't save the request id in async_send's callback.
				// The following will occurs when using async_send with callback :
				// 1. call async_send with callback
				// 2. recv response by rpc_recv_op
				// 3. the callback was called
				// It means that : The callback function of async_send may be called after 
				// recved response data.

				asio::post(derive.io_->context(), make_allocator(derive.callocator_, std::move(fn)));
			}
		};

		template<class derive_t>
		class sync_caller
		{
			template <class, class>                       friend class rpc_call_cp;
		protected:
			sync_caller(derive_t& d) noexcept
				: derive(d), id_(0), tm_(d.get_default_timeout()) {}
			sync_caller(sync_caller&& o) noexcept
				: derive(o.derive), id_(std::move(o.id_)), tm_(std::move(o.tm_)) {}

			sync_caller(const sync_caller&) = delete;
			sync_caller& operator=(sync_caller&&) = delete;
			sync_caller& operator=(const sync_caller&) = delete;

		public:
			~sync_caller() = default;

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call.
			 */
			template<class Rep, class Period>
			inline sync_caller& set_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call. same as set_timeout
			 */
			template<class Rep, class Period>
			inline sync_caller& timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				return this->set_timeout(std::move(timeout));
			}

			// If invoke synchronization rpc call function in communication thread, it will degenerates
			// into async_call and the return value is empty.
			template<class return_t, class ...Args>
			inline return_t call(std::string name, Args&&... args)
			{
				return sync_call_op<derive_t>::template exec<return_t>(
					derive, tm_, std::move(name), std::forward<Args>(args)...);
			}

		protected:
			derive_t&                                           derive;
			rpc_header::id_type                                 id_;
			asio::steady_timer::duration                        tm_;
		};

		template<class derive_t>
		class async_caller
		{
			template <class, class>                       friend class rpc_call_cp;
		protected:
			async_caller(derive_t& d) noexcept
				: derive(d), id_(0), tm_(d.get_default_timeout()) {}
			async_caller(async_caller&& o) noexcept
				: derive(o.derive)
				, id_(std::move(o.id_)), tm_(std::move(o.tm_))
				, cb_(std::move(o.cb_)), fn_(std::move(o.fn_)) {}

			async_caller(const async_caller&) = delete;
			async_caller& operator=(async_caller&&) = delete;
			async_caller& operator=(const async_caller&) = delete;

			using defer_fn = std::function<void(rpc_header::id_type, asio::steady_timer::duration,
				std::function<void(error_code, std::string_view)>)>;

		public:
			~async_caller()
			{
				if (this->fn_)
				{
					(this->fn_)(std::move(this->id_), std::move(this->tm_), std::move(this->cb_));
				}
			}

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call.
			 */
			template<class Rep, class Period>
			inline async_caller& set_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				this->tm_ = timeout;
				return (*this);
			}

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call. same as set_timeout
			 */
			template<class Rep, class Period>
			inline async_caller& timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				return this->set_timeout(std::move(timeout));
			}

			/**
			 * @brief Set the callback function of this rpc call, only valid for this once call.
			 */
			template<class Callback>
			inline async_caller& response(Callback&& cb)
			{
				this->id_ = derive.mkid();
				this->cb_ = async_call_op<derive_t>::template make_callback(derive, std::forward<Callback>(cb));
				return (*this);
			}

			template<class ...Args>
			inline async_caller& async_call(std::string name, Args&&... args)
			{
				derive_t& deriv = derive;
				this->fn_ = [&deriv, req = rpc_request<Args...>{ std::move(name),std::forward<Args>(args)... }]
				(rpc_header::id_type id, asio::steady_timer::duration timeout,
					std::function<void(error_code, std::string_view)> cb) mutable
				{
					if (!id)
					{
						async_call_op<derive_t>::template exec(deriv, std::move(req));
					}
					else
					{
						if (!cb)
						{
							async_call_op<derive_t>::template exec(deriv, std::move(id), std::move(timeout),
								[](error_code, std::string_view) {}, std::move(req));
						}
						else
						{
							async_call_op<derive_t>::template exec(deriv, std::move(id), std::move(timeout),
								std::move(cb), std::move(req));
						}
					}
				};

				return (*this);
			}

		protected:
			derive_t&                                                derive;
			rpc_header::id_type                                      id_;
			asio::steady_timer::duration                             tm_;
			std::function<void(error_code, std::string_view)>        cb_;
			defer_fn                                                 fn_;
		};

		template<class derive_t>
		class base_caller
		{
			template <class, class>                       friend class rpc_call_cp;
		protected:
			base_caller(derive_t& d) noexcept
				: derive(d), tm_(d.get_default_timeout()) {}
			base_caller(base_caller&& o) noexcept
				: derive(o.derive), tm_(std::move(o.tm_)) {}

			base_caller& operator=(base_caller&&) = delete;
			base_caller(const base_caller&) = delete;
			base_caller& operator=(const base_caller&) = delete;

		public:
			~base_caller() = default;

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call.
			 */
			template<class Rep, class Period>
			inline base_caller& set_timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			/**
			 * @brief Set the timeout of this rpc call, only valid for this once call. same as set_timeout
			 */
			template<class Rep, class Period>
			inline base_caller& timeout(std::chrono::duration<Rep, Period> timeout) noexcept
			{
				return this->set_timeout(std::move(timeout));
			}

			/**
			 * @brief Set the callback function of this rpc call, only valid for this once call.
			 */
			template<class Callback>
			inline async_caller<derive_t> response(Callback&& cb)
			{
				async_caller<derive_t> caller{ derive };
				caller.set_timeout(std::move(this->tm_));
				caller.response(std::forward<Callback>(cb));
				return caller; // "caller" is local variable has RVO optimization, should't use std::move()
			}

			// If invoke synchronization rpc call function in communication thread, it will degenerates
			// into async_call and the return value is empty.
			template<class return_t, class ...Args>
			inline return_t call(std::string name, Args&&... args)
			{
				return sync_call_op<derive_t>::template exec<return_t>(derive, this->tm_,
					std::move(name), std::forward<Args>(args)...);
			}

			template<class ...Args>
			inline async_caller<derive_t> async_call(std::string name, Args&&... args)
			{
				async_caller<derive_t> caller{ derive };
				caller.set_timeout(std::move(this->tm_));
				caller.async_call(std::move(name), std::forward<Args>(args)...);
				return caller; // "caller" is local variable has RVO optimization, should't use std::move()
			}

		protected:
			derive_t&                                           derive;
			asio::steady_timer::duration                        tm_;
		};

	public:
		/**
		 * @brief call a rpc function
		 * If invoke synchronization rpc call function in communication thread, it will degenerates
		 * into async_call and the return value is empty.
		 * You can use get_last_error to check whether there is an error of the call
		 */
		template<class return_t, class Rep, class Period, class ...Args>
		inline return_t call(std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t>::template exec<return_t>(derive, timeout,
				std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @brief call a rpc function
		 * If invoke synchronization rpc call function in communication thread, it will degenerates
		 * into async_call and the return value is empty.
		 * You can use get_last_error to check whether there is an error of the call
		 */
		template<class return_t, class ...Args>
		inline return_t call(std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return sync_call_op<derived_t>::template exec<return_t>(derive,
				derive.get_default_timeout(), std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @brief asynchronous call a rpc function
		 * Callback signature : void(DataType result)  example : [](std::string result){}
		 * if result type is void, the Callback signature is : void()
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda.
		 */
		template<class Callback, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(Callback&& cb, std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t>::template exec(derive, derive.mkid(), derive.get_default_timeout(),
				async_call_op<derived_t>::template make_callback(derive, std::forward<Callback>(cb)),
				rpc_request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @brief asynchronous call a rpc function
		 * Callback signature : void(DataType result)  example : [](std::string result){}
		 * if result type is void, the Callback signature is : void()
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda
		 */
		template<class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(Callback&& cb, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t>::template exec(derive, derive.mkid(), timeout,
				async_call_op<derived_t>::template make_callback(derive, std::forward<Callback>(cb)),
				rpc_request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @brief asynchronous call a rpc function
		 * Callback signature : void(return_t result)
		 * the return_t is the first template parameter.
		 * if result type is void, the Callback signature is : void()
		 */
		template<class return_t, class Callback, class ...Args>
		inline typename std::enable_if_t<detail::is_template_callable_v<Callback, return_t>, void>
		async_call(Callback&& cb, std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t>::template exec(derive, derive.mkid(), derive.get_default_timeout(),
				async_call_op<derived_t>::template make_callback<return_t>(
					derive, std::forward<Callback>(cb)),
				rpc_request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @brief asynchronous call a rpc function
		 * Callback signature : void(return_t result)
		 * the return_t is the first template parameter.
		 * if result type is void, the Callback signature is : void()
		 */
		template<class return_t, class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<detail::is_template_callable_v<Callback, return_t>, void>
		async_call(Callback&& cb, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			async_call_op<derived_t>::template exec(derive, derive.mkid(), timeout,
				async_call_op<derived_t>::template make_callback<return_t>(
					derive, std::forward<Callback>(cb)),
				rpc_request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @brief asynchronous call a rpc function
		 */
		template<class ...Args>
		inline async_caller<derived_t> async_call(std::string name, Args&&... args)
		{
			async_caller<derived_t> caller{ static_cast<derived_t&>(*this) };
			caller.async_call(std::move(name), std::forward<Args>(args)...);
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @brief Set the timeout of this rpc call, only valid for this once call.
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t> set_timeout(std::chrono::duration<Rep, Period> timeout)
		{
			base_caller<derived_t> caller{ static_cast<derived_t&>(*this) };
			caller.set_timeout(timeout);
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

		/**
		 * @brief Set the timeout of this rpc call, only valid for this once call. same as set_timeout
		 */
		template<class Rep, class Period>
		inline base_caller<derived_t> timeout(std::chrono::duration<Rep, Period> timeout)
		{
			return this->set_timeout(std::move(timeout));
		}

		/**
		 * @brief Set the callback function of this rpc call, only valid for this once call.
		 */
		template<class Callback>
		inline async_caller<derived_t> response(Callback&& cb)
		{
			async_caller<derived_t> caller{ static_cast<derived_t&>(*this) };
			caller.response(std::forward<Callback>(cb));
			return caller; // "caller" is local variable has RVO optimization, should't use std::move()
		}

	protected:
		rpc_serializer    & sr_;
		rpc_deserializer  & dr_;

		/// The memory to use for handler-based custom memory allocation. used fo async_call.
		handler_memory<std::false_type, assizer<args_t>>   callocator_;

		std::map<rpc_header::id_type, std::function<void(error_code, std::string_view)>> reqs_;
	};
}

#endif // !__ASIO2_RPC_CALL_COMPONENT_HPP__
