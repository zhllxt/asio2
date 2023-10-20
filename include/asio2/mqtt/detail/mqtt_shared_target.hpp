/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 * 
 * refrenced from : mqtt_cpp/include/mqtt/broker/shared_target.hpp
 */

#ifndef __ASIO2_MQTT_SHARED_TARGET_HPP__
#define __ASIO2_MQTT_SHARED_TARGET_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <optional>
#include <thread>
#include <map>
#include <unordered_map>
#include <chrono>
#include <set>
#include <vector>

#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	// v is shared target node
	template<typename STNode>
	auto round_shared_target_method(STNode& v) -> 
		std::shared_ptr<typename asio2::detail::remove_cvref_t<STNode>::session_type>
	{
		using session_type = typename asio2::detail::remove_cvref_t<STNode>::session_type;

		std::weak_ptr<session_type> session;

		if (!v.session_map.empty())
		{
			if (v.last == v.session_map.end())
				v.last = v.session_map.begin();
			else
			{
				v.last = std::next(v.last);

				if (v.last == v.session_map.end())
					v.last = v.session_map.begin();
			}

			session = v.last->second;
		}

		return session.lock();
	}

	template<typename STNode>
	class shared_target
	{
	public:
		struct hasher
		{
			inline std::size_t operator()(std::pair<std::string_view, std::string_view> const& pair) const noexcept
			{
				std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
					(const unsigned char*)(pair.first.data()), pair.first.size());
				return asio2::detail::fnv1a_hash<std::size_t>(v,
					(const unsigned char*)(pair.second.data()), pair.second.size());
			}
		};

		shared_target()
		{
			set_policy(std::bind(round_shared_target_method<STNode>, std::placeholders::_1));
		}
		~shared_target() = default;

		using session_t    = typename STNode::session_type;
		using session_type = typename STNode::session_type;

		template<class Function>
		inline shared_target& set_policy(Function&& fun)
		{
			asio2::unique_locker g(this->shared_target_mutex_);

			policy_ = std::forward<Function>(fun);

			return (*this);
		}

	public:
		void insert(std::shared_ptr<session_t>& session, std::string_view share_name, std::string_view topic_filter)
		{
			auto key = std::pair{ share_name, topic_filter };

			asio2::unique_locker g(this->shared_target_mutex_);

			auto it = targets_.find(key);
			if (it == targets_.end())
			{
				STNode v{ share_name, topic_filter };

				auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
					std::chrono::steady_clock::now().time_since_epoch()).count();

				session->shared_target_key_ = ns;

				v.session_map.emplace(ns, session);

				v.session_set.emplace(session.get());

				key = std::pair{ v.share_name_view(), v.topic_filter_view() };

				it = targets_.emplace(std::move(key), std::move(v)).first;
			}
			else
			{
				STNode& v = it->second;

				if (v.session_set.find(session.get()) != v.session_set.end())
					return;

				for (;;)
				{
					auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
						std::chrono::steady_clock::now().time_since_epoch()).count();

					auto it_map = v.session_map.find(ns);
					if (it_map != v.session_map.end())
					{
						std::this_thread::yield();
						continue;
					}

					session->shared_target_key_ = ns;

					v.session_map.emplace(ns, session);

					v.session_set.emplace(session.get());

					break;
				}
			}
		}
		void erase(std::shared_ptr<session_t>& session, std::string_view share_name, std::string_view topic_filter)
		{
			auto key = std::pair{ share_name, topic_filter };

			asio2::unique_locker g(this->shared_target_mutex_);

			auto it = targets_.find(key);
			if (it == targets_.end())
				return;

			STNode& v = it->second;

			auto it_map = v.session_map.find(session->shared_target_key_);
			if (it_map != v.session_map.end())
			{
				if (v.last == it_map)
				{
					if (it_map != v.session_map.begin())
						v.last = std::prev(v.last);
					else
						v.last = std::next(v.last);
				}

				v.session_map.erase(it_map);
			}

			auto it_set = v.session_set.find(session.get());
			if (it_set != v.session_set.end())
			{
				v.session_set.erase(it_set);
			}
		}
		//void erase(std::shared_ptr<session_t>& session, std::set<std::string_view> share_names)
		//{
		//	for (std::string_view share_name : share_names)
		//	{
		//		auto it = targets_.find(share_name);
		//		if (it == targets_.end())
		//			continue;

		//		std::unordered_map<std::string_view, entry>& map_inner = it->second;
		//		auto it_map = map_inner.find(session->client_id());
		//		if (it_map == map_inner.end())
		//			continue;

		//		map_inner.erase(it_map);
		//	}
		//}
		std::shared_ptr<session_t> get_target(std::string_view share_name, std::string_view topic_filter)
		{
			auto key = std::pair{ share_name, topic_filter };

			asio2::unique_locker g(this->shared_target_mutex_);

			auto it = targets_.find(key);
			if (it == targets_.end())
				return std::shared_ptr<session_t>();

			STNode& v = it->second;

			return policy_(v);
		}

	protected:
		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer  shared_target_mutex_;

		/// key : share_name - topic_filter, val : shared target node
		std::unordered_map<std::pair<std::string_view, std::string_view>, STNode, hasher> targets_ ASIO2_GUARDED_BY(shared_target_mutex_);

		std::function<std::shared_ptr<session_type>(STNode&)>                             policy_  ASIO2_GUARDED_BY(shared_target_mutex_);
	};

	template<class session_t>
	struct stnode
	{
		template <class> friend class mqtt::shared_target;

		using session_type = session_t;

		explicit stnode(std::string_view _share_name, std::string_view _topic_filter)
		{
			share_name.resize(_share_name.size());
			std::memcpy((void*)share_name.data(), (const void*)_share_name.data(), _share_name.size());

			topic_filter.resize(_topic_filter.size());
			std::memcpy((void*)topic_filter.data(), (const void*)_topic_filter.data(), _topic_filter.size());

			last = session_map.end();
		}

		inline std::string_view share_name_view()
		{
			return std::string_view{ share_name.data(), share_name.size() };
		}

		inline std::string_view topic_filter_view()
		{
			return std::string_view{ topic_filter.data(), topic_filter.size() };
		}

		std::vector<char> share_name  ; // vector has no SSO
		std::vector<char> topic_filter;

		/// session map ordered by steady_clock
		std::map<std::chrono::nanoseconds::rep, std::weak_ptr<session_t>> session_map;

		/// session unique
		std::set<session_t*>                                              session_set;

		/// last session for shared subscribe
		typename std::map<std::chrono::nanoseconds::rep, std::weak_ptr<session_t>>::iterator last;
	};
}

#endif // !__ASIO2_MQTT_SHARED_TARGET_HPP__
