/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/function_traits.hpp>

#include <asio2/rpc/detail/serialization.hpp>
#include <asio2/rpc/detail/protocol.hpp>
#include <asio2/rpc/detail/invoker.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class rpc_call_cp
	{
	public:
		/**
		 * @constructor
		 */
		rpc_call_cp(io_t&, serializer& sr, deserializer& dr)
			: derive(static_cast<derived_t&>(*this)), sr_(sr), dr_(dr)
		{
		}

		/**
		 * @destructor
		 */
		~rpc_call_cp() = default;

	protected:
		template<class derive_t>
		struct sync_call_op
		{
			template<class return_t, class Rep, class Period, class ...Args>
			inline static return_t exec(derive_t& derive, error_code* ec_ptr,
				std::chrono::duration<Rep, Period> timeout,
				std::string name, Args&&... args)
			{
				error_code ec;
				std::shared_ptr<typename result_t<return_t>::type> v =
					std::make_shared<typename result_t<return_t>::type>();
				try
				{
					if (!derive.is_started())
						asio::detail::throw_error(asio::error::not_connected);

					header::id_type id = derive.mkid();
					request<Args...> req(id, std::move(name), std::forward<Args>(args)...);

					std::shared_ptr<std::promise<error_code>> promise =
						std::make_shared<std::promise<error_code>>();
					std::future<error_code> future = promise->get_future();

					auto ex = [&derive, v, id, pm = std::move(promise)]
					(error_code ec, std::string_view s) mutable
					{
						ASIO2_ASSERT(derive.io().strand().running_in_this_thread());

						if (!ec)
						{
							try
							{
								derive.dr_ >> ec;
								if constexpr (!std::is_void_v<return_t>)
								{
									if (!ec)
										derive.dr_ >> (*v);
								}
								else
								{
									std::ignore = v;
								}
							}
							catch (cereal::exception&  ) { ec = asio::error::no_data; }
							catch (system_error     & e) { ec = e.code();             }
							catch (std::exception   &  ) { ec = asio::error::eof;     }
						}
						set_last_error(ec);
						pm->set_value(ec);

						derive.reqs_.erase(id);
					};

					// Make sure we run on the strand
					if (!derive.io().strand().running_in_this_thread())
					{
						derive.post([&derive, p = derive.selfptr(),
							req = std::move(req), ex = std::move(ex)]() mutable
						{
							if (derive.send((derive.sr_.reset() << req).str()))
							{
								derive.reqs_.emplace(req.id(), std::move(ex));
							}
							else
							{
								ex(get_last_error(), std::string_view{});
							}
						});
						std::future_status status = future.wait_for(timeout);
						if (status == std::future_status::ready)
						{
							ec = future.get();
						}
						else
						{
							ec = asio::error::timed_out;
							derive.post([&derive, p = derive.selfptr(), id]() mutable
							{
								derive.reqs_.erase(id);
							});
						}
					}
					else
					{
						// Unable to invoke synchronization rpc call function in communication thread
						ASIO2_ASSERT(false);
						ec = asio::error::operation_not_supported;

						//if (derive.send((derive.sr_.reset() << req).str()))
						//{
						//	// Set ec a default value of timed_out first
						//	ec = asio::error::timed_out;
						//	std::future_status status = std::future_status::timeout;
						//	derive.reqs_.emplace(req.id(), std::move(ex));
						//	auto t1 = std::chrono::steady_clock::now();
						//	for (auto elapsed = t1 - t1; elapsed < timeout; elapsed = std::chrono::steady_clock::now() - t1)
						//	{
						//		derive.io().context().run_for(timeout - elapsed);
						//		status = future.wait_for(std::chrono::nanoseconds(0));
						//		if (status == std::future_status::ready)
						//		{
						//			ec = future.get();
						//			break;
						//		}
						//	}
						//	if (status != std::future_status::ready)
						//		derive.reqs_.erase(req.id());
						//}
						//else
						//{
						//	ec = get_last_error();
						//}
					}
				}
				catch (cereal::exception&  ) { ec = asio::error::no_data; }
				catch (system_error     & e) { ec = e.code(); }
				catch (std::exception   &  ) { ec = asio::error::eof; }

				set_last_error(ec);

				if (ec_ptr)
				{
					*ec_ptr = ec;
				}
				else
				{
					asio::detail::throw_error(ec);
				}

				if constexpr (!std::is_void_v<return_t>)
				{
					return std::move(*v);
				}
				else
				{
					std::ignore = true;
				}
			}
		};

		template<class derive_t>
		struct async_call_op
		{
			template<class return_t, class Callback>
			inline static std::function<void(error_code, std::string_view)>
				make_callback(derive_t& derive, Callback&& cb)
			{
				return async_call_op<derive_t>::template make_callback_impl<return_t>(
					derive, std::forward<Callback>(cb));
			}

			template<class Callback>
			inline static std::function<void(error_code, std::string_view)>
				make_callback(derive_t& derive, Callback&& cb)
			{
				using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
				return async_call_op<derive_t>::template make_callback_argc<fun_traits_type::argc>(
					derive, std::forward<Callback>(cb));
			}

			template<std::size_t Argc, class Callback>
			typename std::enable_if_t<Argc == 1, std::function<void(error_code, std::string_view)>>
			inline static make_callback_argc(derive_t& derive, Callback&& cb)
			{
				return async_call_op<derive_t>::template make_callback_impl<void>(
					derive, std::forward<Callback>(cb));
			}

			template<std::size_t Argc, class Callback>
			typename std::enable_if_t<Argc == 2, std::function<void(error_code, std::string_view)>>
			inline static make_callback_argc(derive_t& derive, Callback&& cb)
			{
				using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
				using return_type = typename fun_traits_type::template args<1>::type;
				static_assert(!std::is_same_v<return_type, void>);
				return async_call_op<derive_t>::template make_callback_impl<return_type>(
					derive, std::forward<Callback>(cb));
			}

			template<class return_t, class Callback>
			typename std::enable_if_t<std::is_same_v<return_t, void>,
				std::function<void(error_code, std::string_view)>>
			inline static make_callback_impl(derive_t& derive, Callback&& cb)
			{
				return std::function<void(error_code, std::string_view)>
				{
					[&derive, cb = std::forward<Callback>(cb)](auto ec, std::string_view) mutable
					{
						try
						{
							if (!ec)
								derive.dr_ >> ec;
						}
						catch (cereal::exception&  ) { ec = asio::error::no_data; }
						catch (system_error     & e) { ec = e.code();             }
						catch (std::exception   &  ) { ec = asio::error::eof;     }

						set_last_error(ec);

						cb(ec);
					}
				};
			}

			template<class return_t, class Callback>
			typename std::enable_if_t<!std::is_same_v<return_t, void>,
				std::function<void(error_code, std::string_view)>>
			inline static make_callback_impl(derive_t& derive, Callback&& cb)
			{
				return std::function<void(error_code, std::string_view)>
				{
					[&derive, cb = std::forward<Callback>(cb)](auto ec, std::string_view s) mutable
					{
						typename result_t<return_t>::type v{};

						try
						{
							if (!ec)
								derive.dr_ >> ec;

							if (!ec)
								derive.dr_ >> v;
						}
						catch (cereal::exception&  ) { ec = asio::error::no_data; }
						catch (system_error     & e) { ec = e.code();             }
						catch (std::exception   &  ) { ec = asio::error::eof;     }

						set_last_error(ec);

						cb(ec, std::move(v));
					}
				};
			}

			template<class Req>
			inline static void exec(derive_t& derive, Req&& req)
			{
				ASIO2_ASSERT(!req.id());

				error_code ec;

				try
				{
					if (!derive.is_started())
						asio::detail::throw_error(asio::error::not_connected);

					auto task = [&derive, p = derive.selfptr(), req = std::move(req)]() mutable
					{
						derive.send((derive.sr_.reset() << req).str());
					};

					derive.post(std::move(task));

					return;
				}
				catch (cereal::exception&  ) { ec = asio::error::no_data; }
				catch (system_error     & e) { ec = e.code();             }
				catch (std::exception   &  ) { ec = asio::error::eof;     }

				set_last_error(ec);
			}

			template<class Callback, class Rep, class Period, class Req>
			inline static void exec(derive_t& derive, header::id_type id,
				std::chrono::duration<Rep, Period> timeout, Callback&& cb, Req&& req)
			{
				ASIO2_ASSERT(id);

				error_code ec;

				req.id(id);

				copyable_wrapper<asio::steady_timer> timer(derive.io().context());
				timer().expires_after(timeout);
				timer().async_wait(asio::bind_executor(derive.io().strand(), [&derive, id]
				(const error_code & ec) mutable
				{
					if (ec == asio::error::operation_aborted)
						return;
					auto iter = derive.reqs_.find(id);
					if (iter != derive.reqs_.end())
					{
						auto& ex = iter->second;
						ex(asio::error::timed_out, std::string_view{});
					}
				}));

				auto ex = [&derive, id, timer = std::move(timer), cb = std::forward<Callback>(cb)]
				(error_code ec, std::string_view s) mutable
				{
					ASIO2_ASSERT(derive.io().strand().running_in_this_thread());

					timer().cancel(ec_ignore);

					if (cb) { cb(ec, s); }

					derive.reqs_.erase(id);
				};

				try
				{
					if (!derive.is_started())
						asio::detail::throw_error(asio::error::not_connected);

					auto task = [&derive, p = derive.selfptr(),
						req = std::move(req), ex = std::move(ex)]() mutable
					{
						if (derive.send((derive.sr_.reset() << req).str()))
						{
							derive.reqs_.emplace(req.id(), std::move(ex));
						}
						else
						{
							ex(get_last_error(), std::string_view{});
						}
					};

					// 2019-11-28 fixed the bug of issue #6 : task() cannot be called directly
					// Make sure we run on the strand
					//if (!derive.io().strand().running_in_this_thread())
					derive.post(std::move(task));
					//else
					//	task();

					return;
				}
				catch (cereal::exception&  ) { ec = asio::error::no_data; }
				catch (system_error     & e) { ec = e.code();             }
				catch (std::exception   &  ) { ec = asio::error::eof;     }

				set_last_error(ec);

				// bug fixed : can't call ex(...) directly, it will 
				// cause "reqs_.erase(id)" be called in multithread 
				derive.post([ec, p = derive.selfptr(), ex = std::move(ex)]() mutable
				{
					set_last_error(ec);

					ex(ec, std::string_view{});
				});
			}
		};

		template<class derive_t>
		class sync_caller
		{
			template <class, bool>                       friend class rpc_call_cp;
		protected:
			sync_caller(derive_t& d) : derive(d), id_(0), tm_(d.default_timeout()) {}
			sync_caller(sync_caller&& o) : derive(o.derive)
				, id_(std::move(o.id_)), tm_(std::move(o.tm_)), ec_(std::move(o.ec_)) {}
			sync_caller(const sync_caller&) = delete;
			sync_caller& operator=(sync_caller&&) = delete;
			sync_caller& operator=(const sync_caller&) = delete;

		public:
			~sync_caller() = default;

			template<class Rep, class Period>
			inline sync_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			inline sync_caller& errcode(error_code& ec)
			{
				this->ec_ = &ec;
				return (*this);
			}

			template<class return_t, class ...Args>
			inline return_t call(std::string name, Args&&... args)
			{
				return sync_call_op<derive_t>::template exec<return_t>(
					derive, ec_, tm_, std::move(name), std::forward<Args>(args)...);
			}

		protected:
			derive_t&                                           derive;
			header::id_type                                     id_;
			asio::steady_timer::duration                        tm_;
			error_code*                                         ec_       = nullptr;
		};

		template<class derive_t>
		class async_caller
		{
			template <class, bool>                       friend class rpc_call_cp;
		protected:
			async_caller(derive_t& d) : derive(d), id_(0), tm_(d.default_timeout()) {}
			async_caller(async_caller&& o) : derive(o.derive)
				, id_(std::move(o.id_)), tm_(std::move(o.tm_))
				, cb_(std::move(o.cb_)), fn_(std::move(o.fn_)) {}
			async_caller(const async_caller&) = delete;
			async_caller& operator=(async_caller&&) = delete;
			async_caller& operator=(const async_caller&) = delete;

			using defer_fn = std::function<void(header::id_type, asio::steady_timer::duration,
				std::function<void(error_code, std::string_view)>)>;

		public:
			~async_caller()
			{
				if (this->fn_)
				{
					(this->fn_)(std::move(this->id_), std::move(this->tm_), std::move(this->cb_));
				}
			}

			template<class Rep, class Period>
			inline async_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = timeout;
				return (*this);
			}

			template<class Callback>
			inline async_caller& response(Callback&& cb)
			{
				this->id_ = derive.mkid();
				this->cb_ = async_call_op<derive_t>::template make_callback(
					derive, std::forward<Callback>(cb));
				return (*this);
			}

			template<class ...Args>
			inline async_caller& async_call(std::string name, Args&&... args)
			{
				derive_t& deriv = derive;
				this->fn_ = [&deriv, req = request<Args...>{ std::move(name),std::forward<Args>(args)... }]
				(header::id_type id, asio::steady_timer::duration timeout,
					std::function<void(error_code, std::string_view)> cb) mutable
				{
					if (!id)
					{
						async_call_op<derive_t>::template exec(deriv, std::move(req));
					}
					else
					{
						async_call_op<derive_t>::template exec(deriv, std::move(id), std::move(timeout),
							std::move(cb), std::move(req));
					}
				};

				return (*this);
			}

		protected:
			derive_t&                                                derive;
			header::id_type                                          id_;
			asio::steady_timer::duration                             tm_;
			std::function<void(error_code, std::string_view)>        cb_;
			defer_fn                                                 fn_;
		};

		template<class derive_t>
		class base_caller
		{
			template <class, bool>                       friend class rpc_call_cp;
		protected:
			base_caller(derive_t& d) : derive(d), tm_(d.default_timeout()) {}
			base_caller(base_caller&& o) : derive(o.derive), tm_(std::move(o.tm_)), ec_(std::move(o.ec_)) {}
			base_caller& operator=(base_caller&&) = delete;
			base_caller(const base_caller&) = delete;
			base_caller& operator=(const base_caller&) = delete;

		public:
			~base_caller() = default;

			template<class Rep, class Period>
			inline base_caller& timeout(std::chrono::duration<Rep, Period> timeout)
			{
				this->tm_ = std::move(timeout);
				return (*this);
			}

			inline sync_caller<derive_t> errcode(error_code& ec)
			{
				sync_caller<derive_t> caller{ derive };
				caller.timeout(std::move(this->tm_));
				caller.errcode(ec);
				return std::move(caller);
			}

			template<class Callback>
			inline async_caller<derive_t> response(Callback&& cb)
			{
				async_caller<derive_t> caller{ derive };
				caller.timeout(std::move(this->tm_));
				caller.response(std::forward<Callback>(cb));
				return std::move(caller);
			}

			template<class return_t, class ...Args>
			inline return_t call(std::string name, Args&&... args)
			{
				return sync_call_op<derive_t>::template exec<return_t>(derive, this->ec_, this->tm_,
					std::move(name), std::forward<Args>(args)...);
			}

			template<class ...Args>
			inline async_caller<derive_t> async_call(std::string name, Args&&... args)
			{
				async_caller<derive_t> caller{ derive };
				caller.timeout(std::move(this->tm_));
				caller.async_call(std::move(name), std::forward<Args>(args)...);
				return std::move(caller);
			}

		protected:
			derive_t&                                           derive;
			asio::steady_timer::duration                        tm_;
			error_code*                                         ec_       = nullptr;
		};

	public:
		/**
		 * @function : call a rpc function
		 */
		template<class return_t, class Rep, class Period, class ...Args>
		inline return_t call(std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			return sync_call_op<derived_t>::template exec<return_t>(derive, nullptr, timeout,
				std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : call a rpc function
		 */
		template<class return_t, class Rep, class Period, class ...Args>
		inline return_t call(error_code& ec, std::chrono::duration<Rep, Period> timeout,
			std::string name, Args&&... args)
		{
			return sync_call_op<derived_t>::template exec<return_t>(derive, &ec, timeout,
				std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : call a rpc function
		 */
		template<class return_t, class ...Args>
		inline return_t call(std::string name, Args&&... args)
		{
			return sync_call_op<derived_t>::template exec<return_t>(derive, nullptr,
				derive.default_timeout(), std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : call a rpc function
		 */
		template<class return_t, class ...Args>
		inline return_t call(error_code& ec, std::string name, Args&&... args)
		{
			return sync_call_op<derived_t>::template exec<return_t>(derive, &ec,
				derive.default_timeout(), std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(asio::error_code ec, int result)
		 * if result type is void, the Callback signature is : void(asio::error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda.
		 */
		template<class Callback, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
			async_call(Callback&& cb, std::string name, Args&&... args)
		{
			async_call_op<derived_t>::template exec(derive, derive.mkid(), derive.default_timeout(),
				async_call_op<derived_t>::template make_callback(derive, std::forward<Callback>(cb)),
				request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(asio::error_code ec, int result)
		 * if result type is void, the Callback signature is : void(asio::error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda
		 */
		template<class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(Callback&& cb, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			async_call_op<derived_t>::template exec(derive, derive.mkid(), timeout,
				async_call_op<derived_t>::template make_callback(derive, std::forward<Callback>(cb)),
				request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(asio::error_code ec, return_t result) the return_t
		 * is the first template parameter.
		 * if result type is void, the Callback signature is : void(asio::error_code ec)
		 */
		template<class return_t, class Callback, class ...Args>
		inline typename std::enable_if_t<is_template_callable_v<Callback, error_code, return_t>, void>
		async_call(Callback&& cb, std::string name, Args&&... args)
		{
			async_call_op<derived_t>::template exec(derive, derive.mkid(), derive.default_timeout(),
				async_call_op<derived_t>::template make_callback<return_t>(
					derive, std::forward<Callback>(cb)),
				request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(asio::error_code ec, return_t result) the return_t
		 * is the first template parameter.
		 * if result type is void, the Callback signature is : void(asio::error_code ec)
		 */
		template<class return_t, class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<is_template_callable_v<Callback, error_code, return_t>, void>
		async_call(Callback&& cb, std::chrono::duration<Rep, Period> timeout,
			std::string name, Args&&... args)
		{
			async_call_op<derived_t>::template exec(derive, derive.mkid(), timeout,
				async_call_op<derived_t>::template make_callback<return_t>(
					derive, std::forward<Callback>(cb)),
				request<Args...>{ std::move(name), std::forward<Args>(args)... });
		}

		/**
		 * @function : asynchronous call a rpc function
		 */
		template<class ...Args>
		inline async_caller<derived_t> async_call(std::string name, Args&&... args)
		{
			async_caller<derived_t> caller{ derive };
			caller.async_call(std::move(name), std::forward<Args>(args)...);
			return std::move(caller);
		}

		template<class Rep, class Period>
		inline base_caller<derived_t> timeout(std::chrono::duration<Rep, Period> timeout)
		{
			base_caller<derived_t> caller{ derive };
			caller.timeout(timeout);
			return std::move(caller);
		}

		inline sync_caller<derived_t> errcode(error_code& ec)
		{
			sync_caller<derived_t> caller{ derive };
			caller.errcode(ec);
			return std::move(caller);
		}

		template<class Callback>
		inline async_caller<derived_t> response(Callback&& cb)
		{
			async_caller<derived_t> caller{ derive };
			caller.response(std::forward<Callback>(cb));
			return std::move(caller);
		}

	protected:
		derived_t     & derive;

		serializer    & sr_;
		deserializer  & dr_;

		std::map<header::id_type, std::function<void(error_code, std::string_view)>> reqs_;
	};
}

#endif // !__ASIO2_RPC_CALL_COMPONENT_HPP__
