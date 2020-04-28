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
		rpc_call_cp(io_t& wio, serializer& sr, deserializer& dr)
			: derive(static_cast<derived_t&>(*this)), wio_(wio), sr_(sr), dr_(dr)
		{
		}

		/**
		 * @destructor
		 */
		~rpc_call_cp() = default;

	public:
		/**
		 * @function : call a rpc function
		 */
		template<class T, class Rep, class Period, class ...Args>
		inline T call(std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			error_code ec;
			if constexpr (std::is_void_v<T>)
			{
				derive.template _do_call<T>(ec, timeout, std::move(name), std::forward<Args>(args)...);
				asio::detail::throw_error(ec);
			}
			else
			{
				T v = derive.template _do_call<T>(ec, timeout, std::move(name), std::forward<Args>(args)...);
				asio::detail::throw_error(ec);
				return v;
			}
		}

		/**
		 * @function : call a rpc function
		 */
		template<class T, class Rep, class Period, class ...Args>
		inline T call(error_code& ec, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			if constexpr (std::is_void_v<T>)
			{
				derive.template _do_call<T>(ec, timeout, std::move(name), std::forward<Args>(args)...);
			}
			else
			{
				return derive.template _do_call<T>(ec, timeout, std::move(name), std::forward<Args>(args)...);
			}
		}

		/**
		 * @function : call a rpc function
		 */
		template<class T, class ...Args>
		inline T call(std::string name, Args&&... args)
		{
			error_code ec;
			if constexpr (std::is_void_v<T>)
			{
				derive.template _do_call<T>(ec, derive.timeout(), std::move(name), std::forward<Args>(args)...);
				asio::detail::throw_error(ec);
			}
			else
			{
				T v = derive.template _do_call<T>(ec, derive.timeout(), std::move(name), std::forward<Args>(args)...);
				asio::detail::throw_error(ec);
				return v;
			}
		}

		/**
		 * @function : call a rpc function
		 */
		template<class T, class ...Args>
		inline T call(error_code& ec, std::string name, Args&&... args)
		{
			if constexpr (std::is_void_v<T>)
			{
				derive.template _do_call<T>(ec, derive.timeout(), std::move(name), std::forward<Args>(args)...);
			}
			else
			{
				return derive.template _do_call<T>(ec, derive.timeout(), std::move(name), std::forward<Args>(args)...);
			}
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(error_code ec, int result)
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda.
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class Callback, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(Callback&& fn, std::string name, Args&&... args)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
			derive.template _do_async_call<fun_traits_type::argc>(std::forward<Callback>(fn),
				derive.timeout(), std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(error_code ec, int result)
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * Because the result value type is not specified in the first template parameter,
		 * so the result value type must be specified in the Callback lambda
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class Callback, class Rep, class Period, class ...Args>
		inline typename std::enable_if_t<is_callable_v<Callback>, void>
		async_call(Callback&& fn, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
			derive.template _do_async_call<fun_traits_type::argc>(std::forward<Callback>(fn), timeout,
				std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(error_code ec, T result) the T is the first template parameter.
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class T, class Callback, class ...Args>
		inline void async_call(Callback&& fn, std::string name, Args&&... args)
		{
			derive.template _do_async_call<T>(std::forward<Callback>(fn), derive.timeout(),
				std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Callback signature : void(error_code ec, T result) the T is the first template parameter.
		 * if result type is void, the Callback signature is : void(error_code ec)
		 * You must guarantee that the parameter args remain valid until the send operation is called.
		 */
		template<class T, class Callback, class Rep, class Period, class ...Args>
		inline void async_call(Callback&& fn, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			derive.template _do_async_call<T>(std::forward<Callback>(fn), timeout, std::move(name), std::forward<Args>(args)...);
		}

		/**
		 * @function : asynchronous call a rpc function
		 * Don't care whether the call succeeds
		 */
		template<class String, class ...Args>
		inline typename std::enable_if_t<!is_callable_v<String>, void>
		async_call(String&& name, Args&&... args)
		{
			derive._do_async_call(to_string(std::forward<String>(name)), std::forward<Args>(args)...);
		}

	protected:
		template<class T, class Rep, class Period, class ...Args>
		inline T _do_call(error_code& ec, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			std::shared_ptr<typename result_t<T>::type> v = std::make_shared<typename result_t<T>::type>();
			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				header::id_type id = derive.mkid();
				request<Args...> req(id, std::move(name), std::forward<Args>(args)...);

				std::shared_ptr<std::promise<error_code>> promise = std::make_shared<std::promise<error_code>>();
				std::future<error_code> future = promise->get_future();

				auto cb = [this, v, id, pm = std::move(promise)](error_code ec, std::string_view s) mutable
				{
					if (!ec)
					{
						try
						{
							this->dr_ >> ec;
							if constexpr (!std::is_void_v<T>)
							{
								if (!ec)
									this->dr_ >> (*v);
							}
							else
							{
								std::ignore = v;
							}
						}
						catch (cereal::exception&) { ec = asio::error::no_data; }
						catch (system_error & e) { ec = e.code(); }
						catch (std::exception &) { ec = asio::error::eof; }
					}
					set_last_error(ec);
					pm->set_value(ec);

					this->reqs_.erase(id);
				};

				// Make sure we run on the strand
				if (!this->wio_.strand().running_in_this_thread())
				{
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
						[this, p = derive.selfptr(), req = std::move(req), cb = std::move(cb)]() mutable
					{
						if (derive.send((sr_.reset() << req).str()))
						{
							this->reqs_.emplace(req.id(), std::move(cb));
						}
						else
						{
							cb(get_last_error(), std::string_view{});
						}
					}));
					std::future_status status = future.wait_for(timeout);
					if (status == std::future_status::ready)
					{
						ec = future.get();
					}
					else
					{
						ec = asio::error::timed_out;
						asio::post(this->wio_.strand(), make_allocator(derive.wallocator(),
							[this, p = derive.selfptr(), id]()
						{
							this->reqs_.erase(id);
						}));
					}
				}
				else
				{
					// Unable to invoke synchronization rpc call function in communication thread
					ASIO2_ASSERT(false);
					ec = asio::error::operation_not_supported;

					//if (derive.send((sr_.reset() << req).str()))
					//{
					//	// Set ec a default value of timed_out first
					//	ec = asio::error::timed_out;
					//	std::future_status status = std::future_status::timeout;
					//	this->reqs_.emplace(req.id(), std::move(cb));
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
					//		this->reqs_.erase(req.id());
					//}
					//else
					//{
					//	ec = get_last_error();
					//}
				}
			}
			catch (cereal::exception&) { ec = asio::error::no_data; }
			catch (system_error & e) { ec = e.code(); }
			catch (std::exception &) { ec = asio::error::eof; }

			set_last_error(ec);

			if constexpr (!std::is_void_v<T>) { return (*v); }
			else { static_assert(true); }
		}

	protected:
		template<std::size_t Argc, class Callback, class Rep, class Period, class ...Args>
		typename std::enable_if_t<Argc == 1>
			inline _do_async_call(Callback&& fn, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			using return_type = void;
			derive.template _do_async_call<return_type>(std::forward<Callback>(fn), timeout, std::move(name), std::forward<Args>(args)...);
		}

		template<std::size_t Argc, class Callback, class Rep, class Period, class ...Args>
		typename std::enable_if_t<Argc == 2>
			inline _do_async_call(Callback&& fn, std::chrono::duration<Rep, Period> timeout, std::string name, Args&&... args)
		{
			using fun_traits_type = function_traits<std::remove_cv_t<std::remove_reference_t<Callback>>>;
			using return_type = typename fun_traits_type::template args<1>::type;
			derive.template _do_async_call<return_type>(std::forward<Callback>(fn), timeout, std::move(name), std::forward<Args>(args)...);
		}

		template<class T, class Callback, class Rep, class Period, class ...Args>
		inline void _do_async_call(Callback&& fn, std::chrono::duration<Rep, Period> timeout,
			std::string name, Args&&... args)
		{
			error_code ec;

			header::id_type id = derive.mkid();

			copyable_wrapper<asio::steady_timer> timer(this->wio_.context());
			timer().expires_after(timeout);
			timer().async_wait(asio::bind_executor(this->wio_.strand(), [this, id](const error_code & ec)
			{
				if (ec == asio::error::operation_aborted)
					return;
				auto iter = this->reqs_.find(id);
				if (iter != this->reqs_.end())
				{
					auto& cb = iter->second;
					cb(asio::error::timed_out, std::string_view{});
				}
			}));

			auto cb = [this, id, timer = std::move(timer), fn = std::forward<Callback>(fn)]
			(error_code ec, std::string_view s) mutable
			{
				try
				{
					timer().cancel();
				}
				catch (system_error &) {}
				catch (std::exception &) {}

				typename result_t<T>::type v{};
				if (!ec)
				{
					try
					{
						this->dr_ >> ec;
						if constexpr (!std::is_void_v<T>)
						{
							if (!ec)
								this->dr_ >> v;
						}
						else
						{
							std::ignore = v;
						}
					}
					catch (cereal::exception&) { ec = asio::error::no_data; }
					catch (system_error & e) { ec = e.code(); }
					catch (std::exception &) { ec = asio::error::eof; }
				}
				set_last_error(ec);

				if constexpr (std::is_void_v<T>)
				{
					fn(ec);
				}
				else
				{
					fn(ec, std::move(v));
				}

				this->reqs_.erase(id);
			};

			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				request<Args...> req(id, std::move(name), std::forward<Args>(args)...);

				auto task = [this, p = derive.selfptr(), req = std::move(req), cb = std::move(cb)]() mutable
				{
					if (derive.send((sr_.reset() << req).str()))
					{
						this->reqs_.emplace(req.id(), std::move(cb));
					}
					else
					{
						cb(get_last_error(), std::string_view{});
					}
				};

				// 2019-11-28 fixed the bug of issue #6 : task() cannot be called directly
				// Make sure we run on the strand
				//if (!this->wio_.strand().running_in_this_thread())
					asio::post(this->wio_.strand(), make_allocator(derive.wallocator(), std::move(task)));
				//else
				//	task();

				return;
			}
			catch (cereal::exception&) { ec = asio::error::no_data; }
			catch (system_error & e) { ec = e.code(); }
			catch (std::exception &) { ec = asio::error::eof; }

			set_last_error(ec);

			cb(ec, std::string_view{});
		}

		template<class ...Args>
		inline void _do_async_call(std::string name, Args&&... args)
		{
			error_code ec;

			try
			{
				if (!derive.is_started())
					asio::detail::throw_error(asio::error::not_connected);

				request<Args...> req(header::id_type(0), std::move(name), std::forward<Args>(args)...);

				auto task = [this, p = derive.selfptr(), req = std::move(req)]() mutable
				{
					derive.send((sr_.reset() << req).str());
				};

				asio::post(this->wio_.strand(), make_allocator(derive.wallocator(), std::move(task)));

				return;
			}
			catch (cereal::exception&) { ec = asio::error::no_data; }
			catch (system_error & e) { ec = e.code(); }
			catch (std::exception &) { ec = asio::error::eof; }

			set_last_error(ec);
		}

	protected:
		derived_t     & derive;

		io_t          & wio_;

		serializer    & sr_;
		deserializer  & dr_;

		std::map<header::id_type, std::function<void(error_code, std::string_view)>> reqs_;
	};
}

#endif // !__ASIO2_RPC_CALL_COMPONENT_HPP__
