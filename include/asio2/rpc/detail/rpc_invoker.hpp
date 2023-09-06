/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RPC_INVOKER_HPP__
#define __ASIO2_RPC_INVOKER_HPP__

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
#include <optional>

#include <asio2/base/iopool.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>
#include <asio2/rpc/detail/rpc_protocol.hpp>

#include <asio2/util/string.hpp>

namespace asio2::detail
{
	// forward declare
	template<class, class> class rpc_invoker_t;
}

namespace asio2::rpc
{
	template<class T>
	class response_defer
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;

	public:
		 response_defer() = default;
		~response_defer()
		{
			ASIO2_ASSERT(f_);

			if (f_) { f_(); }
		}

		template<class V>
		inline void set_value(V&& v) { v_ = std::forward<V>(v); }

	protected:
		template<class F>
		inline void _bind    (F&& f) { f_ = std::forward<F>(f); }

	protected:
		std::optional<T>      v_{};
		std::function<void()> f_{};
	};

	template<class T>
	class future
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;
	public:
		future() = delete;
		future(std::shared_ptr<response_defer<T>> defer) noexcept : defer_(std::move(defer))
		{
		}
		~future() = default;

		future(future&&) noexcept = default;
		future(future const&) = default;
		future& operator=(future&&) noexcept = default;
		future& operator=(future const&) = default;

	protected:
		std::shared_ptr<response_defer<T>> defer_{};
	};

	template<class T>
	class promise
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;
	public:
		 promise() = default;
		~promise() = default;

		promise(promise&&) noexcept = default;
		promise(promise const&) = default;
		promise& operator=(promise&&) noexcept = default;
		promise& operator=(promise const&) = default;

		inline future<T> get_future() const noexcept { return future<T>{ defer_ }; }

		template<class V>
		inline void set_value(V&& v) { defer_->set_value(std::forward<V>(v)); }

	protected:
		std::shared_ptr<response_defer<T>> defer_ = std::make_shared<response_defer<T>>();
	};

	//---------------------------------------------------------------------------------------------
	// specialize for void
	//---------------------------------------------------------------------------------------------

	template<>
	class response_defer<void>
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;
	public:
		 response_defer() = default;
		~response_defer()
		{
			ASIO2_ASSERT(f_);

			if (f_) { f_(); }
		}

		template<typename = void>
		inline void set_value() { v_ = '0'; }

	protected:
		template<class F>
		inline void _bind    (F&& f) { f_ = std::forward<F>(f); }

	protected:
		std::optional<char>   v_{};
		std::function<void()> f_{};
	};

	template<>
	class future<void>
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;
	public:
		future() = delete;
		future(std::shared_ptr<response_defer<void>> defer) noexcept : defer_(std::move(defer))
		{
		}
		~future() = default;

		future(future&&) noexcept = default;
		future(future const&) = default;
		future& operator=(future&&) noexcept = default;
		future& operator=(future const&) = default;

	protected:
		std::shared_ptr<response_defer<void>> defer_{};
	};

	template<>
	class promise<void>
	{
		template<class, class> friend class asio2::detail::rpc_invoker_t;
	public:
		 promise() = default;
		~promise() = default;

		promise(promise&&) noexcept = default;
		promise(promise const&) = default;
		promise& operator=(promise&&) noexcept = default;
		promise& operator=(promise const&) = default;

		inline future<void> get_future() const noexcept { return future<void>{ defer_ }; }

		template<typename = void>
		inline void set_value() { defer_->set_value(); }

	protected:
		std::shared_ptr<response_defer<void>> defer_ = std::make_shared<response_defer<void>>();
	};
}

namespace asio2::detail
{
	template<class T>
	struct rpc_result_t
	{
		using type = typename std::remove_cv_t<std::remove_reference_t<T>>;
	};

	template<>
	struct rpc_result_t<void>
	{
		using type = std::int8_t;
	};

	template<class caller_t, class args_t>
	class rpc_invoker_t
	{
	protected:
		struct dummy {};

	public:
		using self = rpc_invoker_t<caller_t, args_t>;
		using fntype = std::function<
			bool(std::shared_ptr<caller_t>&, caller_t*, rpc_serializer&, rpc_deserializer&)>;

		/**
		 * @brief constructor
		 */
		rpc_invoker_t() = default;

		/**
		 * @brief destructor
		 */
		~rpc_invoker_t() = default;

		rpc_invoker_t(rpc_invoker_t&& o) noexcept : rpc_invokers_(std::move(o.rpc_invokers_))
		{
		}
		rpc_invoker_t(rpc_invoker_t const& o) : rpc_invokers_(o.rpc_invokers_)
		{
		}
		rpc_invoker_t& operator=(rpc_invoker_t&& o) noexcept
		{
			this->rpc_invokers_ = std::move(o.rpc_invokers_);
		}
		rpc_invoker_t& operator=(rpc_invoker_t const& o)
		{
			this->rpc_invokers_ = o.rpc_invokers_;
		}

		/**
		 * @brief bind a rpc function
		 * @param name - Function name in string format.
		 * @param fun - Function object.
		 * @param obj - A pointer or reference to a class object, this parameter can be none.
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or reference.
		 */
		template<class F, class ...C>
		inline self& bind(std::string name, F&& fun, C&&... obj)
		{
			asio2::trim_both(name);

			ASIO2_ASSERT(!name.empty());
			if (name.empty())
				return (*this);

		#if defined(_DEBUG) || defined(DEBUG)
			{
			#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
				asio2::shared_locker guard(this->rpc_invoker_mutex_);
			#endif
				ASIO2_ASSERT(this->rpc_invokers_.find(name) == this->rpc_invokers_.end());
			}
		#endif

			this->_bind(std::move(name), std::forward<F>(fun), std::forward<C>(obj)...);

			return (*this);
		}

		/**
		 * @brief unbind a rpc function
		 */
		inline self& unbind(std::string const& name)
		{
		#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			asio2::unique_locker guard(this->rpc_invoker_mutex_);
		#endif
			this->rpc_invokers_.erase(name);

			return (*this);
		}

		/**
		 * @brief find binded rpc function by name
		 */
		inline std::shared_ptr<fntype> find(std::string const& name)
		{
		#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			asio2::shared_locker guard(this->rpc_invoker_mutex_);
		#endif
			if (auto iter = this->rpc_invokers_.find(name); iter != this->rpc_invokers_.end())
				return iter->second;
			return nullptr;
		}

	protected:
		inline self& _invoker() noexcept { return (*this); }
		inline self const& _invoker() const noexcept { return (*this); }

		template<class F>
		inline void _bind(std::string name, F f)
		{
		#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			asio2::unique_locker guard(this->rpc_invoker_mutex_);
		#endif
			this->rpc_invokers_[std::move(name)] = std::make_shared<fntype>(std::bind(&self::template _proxy<F, dummy>,
				this, std::move(f), nullptr,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		}

		template<class F, class C>
		inline void _bind(std::string name, F f, C& c)
		{
		#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			asio2::unique_locker guard(this->rpc_invoker_mutex_);
		#endif
			this->rpc_invokers_[std::move(name)] = std::make_shared<fntype>(std::bind(&self::template _proxy<F, C>,
				this, std::move(f), std::addressof(c),
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		}

		template<class F, class C>
		inline void _bind(std::string name, F f, C* c)
		{
		#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			asio2::unique_locker guard(this->rpc_invoker_mutex_);
		#endif
			this->rpc_invokers_[std::move(name)] = std::make_shared<fntype>(std::bind(&self::template _proxy<F, C>,
				this, std::move(f), c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
		}

		template<class F, class C>
		inline bool _proxy(F& f, C* c, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			rpc_serializer& sr, rpc_deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;

			return _argc_proxy<fun_traits_type::argc>(f, c, caller_ptr, caller, sr, dr);
		}

		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 0, bool>
		inline _argc_proxy(F& f, C* c, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			rpc_serializer& sr, rpc_deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;

			fun_args_tuple tp;
			detail::for_each_tuple(tp, [&dr](auto& elem) mutable
			{
				dr >> elem;
			});

			return _invoke<fun_ret_type>(f, c, caller_ptr, caller, sr, dr, std::move(tp));
		}

		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc != 0, bool>
		inline _argc_proxy(F& f, C* c, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			rpc_serializer& sr, rpc_deserializer& dr)
		{
			detail::ignore_unused(caller);

			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<
				typename fun_traits_type::template args<0>::type>>;

			if constexpr /**/ (std::is_same_v<arg0_type, std::shared_ptr<caller_t>>)
			{
				auto tp = _body_args_tuple((fun_args_tuple*)0);
				detail::for_each_tuple(tp, [&dr](auto& elem) mutable
				{
					dr >> elem;
				});
				auto tp_new = std::tuple_cat(std::tuple<std::shared_ptr<caller_t>&>(caller_ptr), tp);
				return _invoke<fun_ret_type>(f, c, caller_ptr, caller, sr, dr, std::move(tp_new));
			}
			else if constexpr (std::is_same_v<arg0_type, caller_t>)
			{
				auto tp = _body_args_tuple((fun_args_tuple*)0);
				detail::for_each_tuple(tp, [&dr](auto& elem) mutable
				{
					dr >> elem;
				});
				auto tp_new = std::tuple_cat(std::tuple<caller_t&>(*caller), tp);
				return _invoke<fun_ret_type>(f, c, caller_ptr, caller, sr, dr, std::move(tp_new));
			}
			else
			{
				fun_args_tuple tp;
				detail::for_each_tuple(tp, [&dr](auto& elem) mutable
				{
					dr >> elem;
				});
				return _invoke<fun_ret_type>(f, c, caller_ptr, caller, sr, dr, std::move(tp));
			}
		}

		template<typename... Args>
		inline decltype(auto) _body_args_tuple(std::tuple<Args...>* tp)
		{
			return (_body_args_tuple_impl(std::make_index_sequence<sizeof...(Args) - 1>{}, tp));
		}

		template<std::size_t... I, typename... Args>
		inline decltype(auto) _body_args_tuple_impl(std::index_sequence<I...>, std::tuple<Args...>*) noexcept
		{
			return (std::tuple<typename std::tuple_element<I + 1, std::tuple<Args...>>::type...>{});
		}

		template<typename R, typename F, typename C>
		inline bool _invoke_with_future(F& f, C* c, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			rpc_serializer& sr, rpc_deserializer& dr, typename rpc_result_t<R>::type r)
		{
			detail::ignore_unused(f, c, caller_ptr, caller, sr, dr);

			error_code ec = rpc::make_error_code(rpc::error::success);

			if (dr.buffer().in_avail() != 0)
			{
				ec = rpc::make_error_code(rpc::error::invalid_argument);
			}

			auto* defer = r.defer_.get();

			detail::io_context_work_guard iocg(caller->io_->context().get_executor());

			r.defer_->_bind(
			[caller_ptr, caller, &sr, ec, head = caller->header_, defer, iocg = std::move(iocg)]() mutable
			{
				detail::ignore_unused(caller_ptr, iocg);

				if (head.id() == static_cast<rpc_header::id_type>(0))
					return;

				// the "header_, async_send" should not appear in this "invoker" module, But I thought 
				// for a long time and couldn't find of a good method to solve this problem.

				// the operator for "sr" must be in the io_context thread. 
				asio::dispatch(caller->io_->context(), make_allocator(caller->wallocator(),
				[caller_ptr = std::move(caller_ptr), caller, &sr, ec, head = std::move(head),
					v = std::move(defer->v_)]
				() mutable
				{
					ASIO2_ASSERT(caller->io_->running_in_this_thread());

					if (!caller->is_started())
						return;

					head.type(rpc_type_rep);

					if (v.has_value() == false && (!ec))
					{
						ec = rpc::make_error_code(rpc::error::no_data);
					}

					sr.reset();
					sr << head;
					sr << ec;

				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					try
					{
				#endif
						if constexpr (!std::is_same_v<rpc::future<void>, R>)
						{
							if (!ec)
							{
								sr << std::move(v.value()); // maybe throw some exception
							}
						}
						else
						{
							std::ignore = v;
						}

						caller->internal_async_send(std::move(caller_ptr), sr.str());

				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
						return; // not exception, return
					}
					catch (cereal::exception const&)
					{
						if (!ec) ec = rpc::make_error_code(rpc::error::invalid_argument);
					}
					catch (std::exception const&)
					{
						if (!ec) ec = rpc::make_error_code(rpc::error::unspecified_error);
					}

					// the error_code must not be 0.
					ASIO2_ASSERT(ec);

					// code run to here, it means that there has some exception.
					sr.reset();
					sr << head;
					sr << ec;

					caller->internal_async_send(std::move(caller_ptr), sr.str());
				#endif
				}));
			});

			return true;
		}

		// async - return true, sync - return false
		template<typename R, typename F, typename C, typename... Args>
		inline bool _invoke(F& f, C* c, std::shared_ptr<caller_t>& caller_ptr, caller_t* caller,
			rpc_serializer& sr, rpc_deserializer& dr, std::tuple<Args...>&& tp)
		{
			detail::ignore_unused(caller_ptr, caller, sr, dr);

			if (caller_ptr)
			{
				detail::get_current_object<std::shared_ptr<caller_t>>() = caller_ptr;
			}
			else
			{
				detail::get_current_object<caller_t*>() = caller;
			}

			typename rpc_result_t<R>::type r = _invoke_impl<R>(f, c,
				std::make_index_sequence<sizeof...(Args)>{}, std::move(tp));

			if constexpr (detail::is_template_instance_of_v<rpc::future, R>)
			{
				return _invoke_with_future<R>(f, c, caller_ptr, caller, sr, dr, std::move(r));
			}
			else if constexpr (!std::is_same_v<R, void>)
			{
				sr << rpc::make_error_code(rpc::error::success);
				sr << r;

				return false;
			}
			else
			{
				sr << rpc::make_error_code(rpc::error::success);
				std::ignore = r;

				return false;
			}
		}

		template<typename R, typename F, typename C, std::size_t... I, typename... Args>
		typename std::enable_if_t<!std::is_same_v<R, void>, typename rpc_result_t<R>::type>
			inline _invoke_impl(F& f, C* c, std::index_sequence<I...>, std::tuple<Args...>&& tp)
		{
			detail::ignore_unused(c);

			if constexpr (std::is_same_v<detail::remove_cvref_t<C>, dummy>)
				return f(std::get<I>(std::move(tp))...);
			else
				return (c->*f)(std::get<I>(std::move(tp))...);
		}

		template<typename R, typename F, typename C, std::size_t... I, typename... Args>
		typename std::enable_if_t<std::is_same_v<R, void>, typename rpc_result_t<R>::type>
			inline _invoke_impl(F& f, C* c, std::index_sequence<I...>, std::tuple<Args...>&& tp)
		{
			detail::ignore_unused(c);

			if constexpr (std::is_same_v<detail::remove_cvref_t<C>, dummy>)
				f(std::get<I>(std::move(tp))...);
			else
				(c->*f)(std::get<I>(std::move(tp))...);

			return 1;
		}

	protected:
	#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
		mutable asio2::shared_mutexer                            rpc_invoker_mutex_;
	#endif

		std::unordered_map<std::string, std::shared_ptr<fntype>> rpc_invokers_
	#if defined(ASIO2_ENABLE_RPC_INVOKER_THREAD_SAFE)
			ASIO2_GUARDED_BY(rpc_invoker_mutex_)
	#endif
			;
	};
}

#endif // !__ASIO2_RPC_INVOKER_HPP__
