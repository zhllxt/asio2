/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_RDC_INVOKER_HPP__
#define __ASIO2_RDC_INVOKER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <map>
#include <type_traits>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/util/string.hpp>

namespace asio2::detail
{
	template<class SendDataT, class RecvDataT>
	struct rdc_make_callback_t
	{
		using self          = rdc_make_callback_t<SendDataT, RecvDataT>;
		using callback_type = std::function<void(const error_code&, SendDataT, RecvDataT)>;

		/**
		 * @brief bind a rdc function
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
			return std::bind(&self::template _proxy<F, C>, std::move(f), std::addressof(c),
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
			detail::ignore_unused(send_data);

			using fun_traits_type = function_traits<F>;
			using arg_type = typename std::remove_cv_t<typename fun_traits_type::template args<0>::type>;

			set_last_error(ec);

			if constexpr (std::is_reference_v<arg_type>)
			{
				f(recv_data);
			}
			else
			{
				if (ec)
				{
					arg_type result;
					f(std::move(result));
					return;
				}

				if /**/ constexpr (has_stream_operator<arg_type, RecvDataT>::value)
				{
					arg_type result;
					result << recv_data;
					f(std::move(result));
				}
				else if constexpr (has_equal_operator<arg_type, RecvDataT>::value)
				{
					arg_type result;
					result = recv_data;
					f(std::move(result));
				}
				else
				{
					arg_type result{ recv_data };
					f(std::move(result));
				}
			}
		}

		template<class F, class C>
		static inline void _proxy(F& f, C* c, const error_code& ec, SendDataT send_data, RecvDataT recv_data)
		{
			detail::ignore_unused(send_data);

			using fun_traits_type = function_traits<F>;
			using arg_type = typename std::remove_cv_t<typename fun_traits_type::template args<0>::type>;
			
			set_last_error(ec);

			if constexpr (std::is_reference_v<arg_type>)
			{
				(c->*f)(recv_data);
			}
			else
			{
				if (ec)
				{
					arg_type result;
					(c->*f)(std::move(result));
					return;
				}

				if /**/ constexpr (has_stream_operator<arg_type, RecvDataT>::value)
				{
					arg_type result;
					result << recv_data;
					(c->*f)(std::move(result));
				}
				else if constexpr (has_equal_operator<arg_type, RecvDataT>::value)
				{
					arg_type result;
					result = recv_data;
					(c->*f)(std::move(result));
				}
				else
				{
					arg_type result{ recv_data };
					(c->*f)(std::move(result));
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
		using value_type    =          std::tuple<std::shared_ptr<asio::steady_timer>, callback_type>;
		using iterator_type = typename std::multimap<IdT, value_type>::iterator;

		/**
		 * @brief constructor
		 */
		rdc_invoker_t() = default;

		/**
		 * @brief destructor
		 */
		~rdc_invoker_t() = default;

		rdc_invoker_t(rdc_invoker_t&&) = default;
		rdc_invoker_t(rdc_invoker_t const&) = default;
		rdc_invoker_t& operator=(rdc_invoker_t&&) = default;
		rdc_invoker_t& operator=(rdc_invoker_t const&) = default;

		/**
		 * @brief find binded rdc function iterator by id
		 */
		inline auto find(IdT const& id)
		{
			// can't use std::multimap::find
			// std::multimap<Key,T,Compare,Allocator>::find
			// Finds an element with key equivalent to key. If there are several elements
			// with key in the container, any of them may be returned.
			auto it = this->rdc_reqs_.lower_bound(id);

			if (it == this->rdc_reqs_.end())
				return it;

			// [20220402] fix bug
			// Returns an iterator pointing to the first element that is not less than
			// (i.e. greater or equal to) key.
			// when multimap has {2,2} {3,3} if you find key 1, the map will return {2,2}
			if (it->first == id)
				return it;

			return this->rdc_reqs_.end();
		}

		/**
		 * @brief 
		 */
		inline auto emplace(IdT id, std::shared_ptr<asio::steady_timer> timer, callback_type cb)
		{
			// std::multimap<Key,T,Compare,Allocator>::insert
			// inserts value. If the container has elements with equivalent key,
			// inserts at the upper bound of that range.(since C++11)
			return this->rdc_reqs_.insert(std::pair(std::move(id), std::tuple(std::move(timer), std::move(cb))));
		}

		/**
		 * @brief
		 */
		inline auto emplace(IdT key, value_type val)
		{
			return this->rdc_reqs_.insert(std::pair(std::move(key), std::move(val)));
		}

		/**
		 * @brief
		 */
		inline auto end()
		{
			return this->rdc_reqs_.end();
		}

		/**
		 * @brief
		 */
		template<class Iter>
		inline auto erase(Iter iter)
		{
			return this->rdc_reqs_.erase(iter);
		}

		/**
		 * @brief
		 */
		inline std::multimap<IdT, value_type>& reqs() noexcept
		{
			return this->rdc_reqs_;
		}

	protected:
		std::multimap<IdT, value_type> rdc_reqs_;
	};
}

#endif // !__ASIO2_RDC_INVOKER_HPP__
