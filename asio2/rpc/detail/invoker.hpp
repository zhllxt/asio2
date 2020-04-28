/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/rpc/detail/serialization.hpp>
#include <asio2/rpc/detail/protocol.hpp>

namespace asio2::detail
{
	template<class T>
	struct result_t
	{
		using type = T;
	};

	template<>
	struct result_t<void>
	{
		using type = std::int8_t;
	};

	template<typename CallerT>
	class invoker_t
	{
	public:
		using self = invoker_t<CallerT>;

		/**
		 * @constructor
		 */
		invoker_t()
		{
		}

		/**
		 * @destructor
		 */
		~invoker_t() = default;

		/**
		 * @function : bind a rpc function
		 * @param    : name - Function name in string format
		 * @param    : fun - Function object
		 * @param    : obj - A pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 */
		template<class F, class ...C>
		inline self& bind(std::string const& name, F&& fun, C&&... obj)
		{
#if defined(_DEBUG) || defined(DEBUG)
			{
				//std::shared_lock<std::shared_mutex> guard(this->mutex_);
				ASIO2_ASSERT(this->invokers_.find(name) == this->invokers_.end());
			}
#endif
			this->_bind(name, std::forward<F>(fun), std::forward<C>(obj)...);

			return (*this);
		}

		/**
		 * @function : unbind a rpc function
		 */
		inline self& unbind(std::string const& name)
		{
			//std::unique_lock<std::shared_mutex> guard(this->mutex_);
			this->invokers_.erase(name);

			return (*this);
		}

		/**
		 * @function : find binded rpc function by name
		 */
		inline std::function<void(std::shared_ptr<CallerT>&, serializer&, deserializer&)>* find(std::string const& name)
		{
			//std::shared_lock<std::shared_mutex> guard(this->mutex_);
			auto iter = this->invokers_.find(name);
			if (iter == this->invokers_.end())
				return nullptr;
			return (&(iter->second));
		}

	protected:
		inline self& _invoker()
		{
			return (*this);
		}

		template<class F>
		inline void _bind(std::string const& name, F f)
		{
			//std::unique_lock<std::shared_mutex> guard(this->mutex_);
			this->invokers_[name] = std::bind(&self::template _proxy<F>, this, std::move(f),
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F, class C>
		inline void _bind(std::string const& name, F f, C& c)
		{
			//std::unique_lock<std::shared_mutex> guard(this->mutex_);
			this->invokers_[name] = std::bind(&self::template _proxy<F, C>, this, std::move(f), &c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F, class C>
		inline void _bind(std::string const& name, F f, C* c)
		{
			//std::unique_lock<std::shared_mutex> guard(this->mutex_);
			this->invokers_[name] = std::bind(&self::template _proxy<F, C>, this, std::move(f), c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F>
		inline void _proxy(F f, std::shared_ptr<CallerT>& caller, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;

			_argc_proxy<fun_traits_type::argc>(f, caller, sr, dr);
		}

		template<class F, class C>
		inline void _proxy(F f, C* c, std::shared_ptr<CallerT>& caller, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;

			_argc_proxy<fun_traits_type::argc>(f, c, caller, sr, dr);
		}

		template<std::size_t Argc, class F>
		typename std::enable_if_t<Argc == 0>
			inline _argc_proxy(const F& f, std::shared_ptr<CallerT>&, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;

			fun_args_tuple tp;
			dr >> tp;
			_invoke<fun_ret_type>(f, sr, dr, tp);
		}

		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc == 0>
			inline _argc_proxy(const F& f, C* c, std::shared_ptr<CallerT>&, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;

			fun_args_tuple tp;
			dr >> tp;
			_invoke<fun_ret_type>(f, c, sr, dr, tp);
		}

		template<std::size_t Argc, class F>
		typename std::enable_if_t<Argc != 0>
			inline _argc_proxy(const F& f, std::shared_ptr<CallerT>& caller, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<typename fun_traits_type::template args<0>::type>>;

			if constexpr (std::is_same_v<std::shared_ptr<CallerT>, arg0_type>)
			{
				auto tp = _body_args_tuple((fun_args_tuple*)0);
				dr >> tp;
				_invoke<fun_ret_type>(f, sr, dr, std::tuple_cat(std::tuple<std::shared_ptr<CallerT>&>(caller), tp));
			}
			else
			{
				fun_args_tuple tp;
				dr >> tp;
				_invoke<fun_ret_type>(f, sr, dr, tp);
			}
		}

		template<std::size_t Argc, class F, class C>
		typename std::enable_if_t<Argc != 0>
			inline _argc_proxy(const F& f, C* c, std::shared_ptr<CallerT>& caller, serializer& sr, deserializer& dr)
		{
			using fun_traits_type = function_traits<F>;
			using fun_args_tuple = typename fun_traits_type::pod_tuple_type;
			using fun_ret_type = typename fun_traits_type::return_type;
			using arg0_type = typename std::remove_cv_t<std::remove_reference_t<typename fun_traits_type::template args<0>::type>>;

			if constexpr (std::is_same_v<std::shared_ptr<CallerT>, arg0_type>)
			{
				auto tp = _body_args_tuple((fun_args_tuple*)0);
				dr >> tp;
				_invoke<fun_ret_type>(f, c, sr, dr, std::tuple_cat(std::tuple<std::shared_ptr<CallerT>&>(caller), tp));
			}
			else
			{
				fun_args_tuple tp;
				dr >> tp;
				_invoke<fun_ret_type>(f, c, sr, dr, tp);
			}
		}

		template<typename... Args>
		inline decltype(auto) _body_args_tuple(std::tuple<Args...>* tp)
		{
			return (_body_args_tuple_impl(std::make_index_sequence<sizeof...(Args) - 1>{}, tp));
		}

		template<std::size_t... I, typename... Args>
		inline decltype(auto) _body_args_tuple_impl(const std::index_sequence<I...>&, std::tuple<Args...>*)
		{
			return (std::tuple<typename std::tuple_element<I + 1, std::tuple<Args...>>::type...>{});
		}

		template<typename R, typename F, typename... Args>
		inline void _invoke(const F& f, serializer& sr, deserializer& dr, const std::tuple<Args...>& tp)
		{
			ignore::unused(dr);
			typename result_t<R>::type r = _invoke_impl<R>(f, std::make_index_sequence<sizeof...(Args)>{}, tp);
			sr << error_code{};
			if constexpr (!std::is_same_v<R, void>)
			{
				sr << r;
			}
			else
			{
				std::ignore = r;
			}
		}

		template<typename R, typename F, typename C, typename... Args>
		inline void _invoke(const F& f, C* c, serializer& sr, deserializer& dr, const std::tuple<Args...>& tp)
		{
			ignore::unused(dr);
			typename result_t<R>::type r = _invoke_impl<R>(f, c, std::make_index_sequence<sizeof...(Args)>{}, tp);
			sr << error_code{};
			if constexpr (!std::is_same_v<R, void>)
			{
				sr << r;
			}
			else
			{
				std::ignore = r;
			}
		}

		template<typename R, typename F, std::size_t... I, typename... Args>
		typename std::enable_if_t<!std::is_same_v<R, void>, typename result_t<R>::type>
			inline _invoke_impl(const F& f, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
		{
			return f(std::get<I>(tp)...);
		}

		template<typename R, typename F, std::size_t... I, typename... Args>
		typename std::enable_if_t<std::is_same_v<R, void>, typename result_t<R>::type>
			inline _invoke_impl(const F& f, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
		{
			f(std::get<I>(tp)...);
			return 1;
		}

		template<typename R, typename F, typename C, std::size_t... I, typename... Args>
		typename std::enable_if_t<!std::is_same_v<R, void>, typename result_t<R>::type>
			inline _invoke_impl(const F& f, C* c, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
		{
			return (c->*f)(std::get<I>(tp)...);
		}

		template<typename R, typename F, typename C, std::size_t... I, typename... Args>
		typename std::enable_if_t<std::is_same_v<R, void>, typename result_t<R>::type>
			inline _invoke_impl(const F& f, C* c, const std::index_sequence<I...>&, const std::tuple<Args...>& tp)
		{
			(c->*f)(std::get<I>(tp)...);
			return 1;
		}

	protected:
		//std::shared_mutex                           mutex_;

		std::unordered_map<std::string, std::function<void(std::shared_ptr<CallerT>&, serializer&, deserializer&)>> invokers_;
	};
}

#endif // !__ASIO2_RPC_INVOKER_HPP__
