/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_CACHE_HPP__
#define __ASIO2_HTTP_CACHE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <cstdint>
#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <map>
#include <unordered_map>
#include <type_traits>

#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	struct enable_cache_t {};

	constexpr static enable_cache_t enable_cache;

	template<bool isRequest, class Body, class Fields = fields>
	inline bool is_cache_enabled(http::message<isRequest, Body, Fields>& msg)
	{
		if constexpr (isRequest)
		{
			if (msg.method() == http::verb::get)
			{
				return true;
			}
			return false;
		}
		else
		{
			return true;
		}
	}
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class caller_t, class args_t, class MessageT>
	class basic_http_cache_t
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	protected:
		struct cache_node
		{
			std::chrono::steady_clock::time_point alive;

			MessageT msg;

			cache_node(std::chrono::steady_clock::time_point t, MessageT m)
				: alive(t), msg(std::move(m))
			{
			}

			inline void update_alive_time() noexcept
			{
				alive = std::chrono::steady_clock::now();
			}
		};

	public:
		using self   = basic_http_cache_t<caller_t, args_t, MessageT>;

		/**
		 * @brief constructor
		 */
		basic_http_cache_t()
		{
		}

		/**
		 * @brief destructor
		 */
		~basic_http_cache_t() = default;

		/**
		 * @brief Add a element into the cache.
		 */
		template<class StringT>
		inline cache_node* emplace(StringT&& url, MessageT msg)
		{
			asio2::unique_locker guard(this->http_cache_mutex_);

			if (this->http_cache_map_.size() >= http_caches_max_count_)
				return nullptr;

			// can't use insert_or_assign, it maybe cause the msg was changed in multithread, and 
			// other thread are using the msg at this time.
			return std::addressof(this->http_cache_map_.emplace(
				detail::to_string(std::forward<StringT>(url)),
				cache_node{ std::chrono::steady_clock::now(), std::move(msg) }).first->second);
		}

		/**
		 * @brief Checks if the cache has no elements.
		 */
		inline bool empty() const noexcept
		{
			asio2::shared_locker guard(this->http_cache_mutex_);

			return this->http_cache_map_.empty();
		}

		/**
		 * @brief Finds the cache with key equivalent to url.
		 */
		template<class StringT>
		inline cache_node* find(const StringT& url)
		{
			asio2::shared_locker guard(this->http_cache_mutex_);

			if (this->http_cache_map_.empty())
				return nullptr;

			// If rehashing occurs due to the insertion, all iterators are invalidated.
			// Otherwise iterators are not affected. References are not invalidated. 
			if constexpr (std::is_same_v<StringT, std::string>)
			{
				if (auto it = this->http_cache_map_.find(url); it != this->http_cache_map_.end())
					return std::addressof(it->second);
			}
			else
			{
				std::string str = detail::to_string(url);
				if (auto it = this->http_cache_map_.find(str); it != this->http_cache_map_.end())
					return std::addressof(it->second);
			}

			return nullptr;
		}

		/**
		 * @brief Set the max number of elements in the container.
		 */
		inline self& set_cache_max_count(std::size_t count) noexcept
		{
			this->http_caches_max_count_ = count;
			return (*this);
		}

		/**
		 * @brief Get the max number of elements in the container.
		 */
		inline std::size_t get_cache_max_count() const noexcept
		{
			return this->http_caches_max_count_;
		}

		/**
		 * @brief Get the current number of elements in the container.
		 */
		inline std::size_t get_cache_count() const noexcept
		{
			asio2::shared_locker guard(this->http_cache_mutex_);

			return this->http_cache_map_.size();
		}

		/**
		 * @brief Requests the removal of expired elements.
		 */
		inline self& shrink_to_fit()
		{
			asio2::unique_locker guard(this->http_cache_mutex_);

			if (this->http_cache_map_.size() < http_caches_max_count_)
				return (*this);

			std::multimap<std::chrono::steady_clock::duration::rep, const std::string*> mms;
			for (auto& [url, node] : this->http_cache_map_)
			{
				mms.emplace(node.alive.time_since_epoch().count(), std::addressof(url));
			}

			std::size_t i = 0, n = mms.size() / 3;
			for (auto& [t, purl] : mms)
			{
				detail::ignore_unused(t);
				if (++i > n)
					break;
				this->http_cache_map_.erase(*purl);
			}

			return (*this);
		}

		/**
		 * @brief Erases all elements from the container.
		 */
		inline self& clear() noexcept
		{
			asio2::unique_locker guard(this->http_cache_mutex_);
			this->http_cache_map_.clear();
			return (*this);
		}

	protected:
		mutable asio2::shared_mutexer               http_cache_mutex_;

		std::unordered_map<std::string, cache_node> http_cache_map_ ASIO2_GUARDED_BY(http_cache_mutex_);

		std::size_t                                 http_caches_max_count_ = 100000;
	};

	template<class caller_t, class args_t>
	using http_cache_t = basic_http_cache_t<caller_t, args_t, http::response<http::flex_body>>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_CACHE_HPP__
