/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RDC_INVOKER_HPP__
#define __ASIO2_RDC_INVOKER_HPP__

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
#include <tuple>
#include <map>
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/function_traits.hpp>

#include <asio2/util/string.hpp>

namespace asio2::detail
{
	template<class, class, class = void>
	struct has_stream_operator : std::false_type {};

	template<class T, class D>
	struct has_stream_operator<T, D, std::void_t<decltype(T{} << D{})>> : std::true_type{};

	template<class, class, class = void>
	struct has_equal_operator : std::false_type {};

	template<class T, class D>
	struct has_equal_operator<T, D, std::void_t<decltype(T{} = D{})>> : std::true_type{};


	template<class SendDataT, class RecvDataT>
	struct rdc_make_callback_t
	{
		using self          = rdc_make_callback_t<SendDataT, RecvDataT>;
		using callback_type = std::function<void(const error_code&, SendDataT, RecvDataT)>;

		/**
		 * @function : bind a rdc function
		 */
		template<class F, class ...C>
		static inline callback_type bind(F&& fun, C&&... obj)
		{
			return self::_bind(std::forward<F>(fun), std::forward<C>(obj)...);
		}

	protected:
		template<class F>
		static inline callback_type _bind(F f)
		{
			return std::bind(&self::template _proxy<F>, std::move(f),
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F, class C>
		static inline callback_type _bind(F f, C& c)
		{
			return std::bind(&self::template _proxy<F, C>, std::move(f), &c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F, class C>
		static inline callback_type _bind(F f, C* c)
		{
			return std::bind(&self::template _proxy<F, C>, std::move(f), c,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
		}

		template<class F>
		static inline void _proxy(F& f, const error_code& ec, SendDataT send_data, RecvDataT recv_data)
		{
			ignore_unused(send_data);

			using fun_traits_type = function_traits<F>;
			using arg1_type = typename std::remove_cv_t<typename fun_traits_type::template args<1>::type>;

			if constexpr (std::is_reference_v<arg1_type>)
			{
				f(ec, recv_data);
			}
			else
			{
				if (ec)
				{
					arg1_type result;
					f(ec, std::move(result));
					return;
				}

				if constexpr (has_stream_operator<arg1_type, RecvDataT>::value)
				{
					arg1_type result;
					result << recv_data;
					f(ec, std::move(result));
				}
				else if constexpr (has_equal_operator<arg1_type, RecvDataT>::value)
				{
					arg1_type result;
					result = recv_data;
					f(ec, std::move(result));
				}
				else
				{
					arg1_type result{ recv_data };
					f(ec, std::move(result));
				}
			}
		}

		template<class F, class C>
		static inline void _proxy(F& f, C* c, const error_code& ec, SendDataT send_data, RecvDataT recv_data)
		{
			ignore_unused(send_data);

			using fun_traits_type = function_traits<F>;
			using arg1_type = typename std::remove_cv_t<typename fun_traits_type::template args<1>::type>;

			if constexpr (std::is_reference_v<arg1_type>)
			{
				f(ec, recv_data);
			}
			else
			{
				if (ec)
				{
					arg1_type result;
					(c->*f)(ec, std::move(result));
					return;
				}

				if constexpr (has_stream_operator<arg1_type, RecvDataT>::value)
				{
					arg1_type result;
					result << recv_data;
					(c->*f)(ec, std::move(result));
				}
				else if constexpr (has_equal_operator<arg1_type, RecvDataT>::value)
				{
					arg1_type result;
					result = recv_data;
					(c->*f)(ec, std::move(result));
				}
				else
				{
					arg1_type result{ recv_data };
					(c->*f)(ec, std::move(result));
				}
			}
		}
	};

	template<class IdT, class SendDataT, class RecvDataT>
	class rdc_invoker_t
	{
	public:
		using self = rdc_invoker_t<IdT, SendDataT, RecvDataT>;
		using callback_type = typename rdc_make_callback_t<SendDataT, RecvDataT>::callback_type;

		/**
		 * @constructor
		 */
		rdc_invoker_t() = default;

		/**
		 * @destructor
		 */
		~rdc_invoker_t() = default;

		rdc_invoker_t(rdc_invoker_t&&) = default;
		rdc_invoker_t(rdc_invoker_t const&) = default;
		rdc_invoker_t& operator=(rdc_invoker_t&&) = default;
		rdc_invoker_t& operator=(rdc_invoker_t const&) = default;

		/**
		 * @function : find binded rdc function iterator by id
		 */
		inline auto find(IdT const& id)
		{
			return this->rdc_reqs_.find(id);
		}

		/**
		 * @function : 
		 */
		inline auto emplace(IdT id, std::shared_ptr<asio::steady_timer> timer, callback_type cb)
		{
			return this->rdc_reqs_.emplace(std::move(id), std::tuple{ std::move(timer), std::move(cb) });
		}

		/**
		 * @function :
		 */
		inline auto end()
		{
			return this->rdc_reqs_.end();
		}

		/**
		 * @function :
		 */
		template<class Iter>
		inline auto erase(Iter iter)
		{
			return this->rdc_reqs_.erase(iter);
		}

		/**
		 * @function :
		 */
		inline std::multimap<IdT, std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>>& reqs()
		{
			return this->rdc_reqs_;
		}

	protected:
		std::multimap<IdT, std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>> rdc_reqs_;
	};
}

#endif // !__ASIO2_RDC_INVOKER_HPP__
