/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/subscription_map.hpp
 */

#ifndef __ASIO2_MQTT_SUBSCRIPTION_MAP_HPP__
#define __ASIO2_MQTT_SUBSCRIPTION_MAP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <vector>
#include <cinttypes>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

#include <asio2/mqtt/idmgr.hpp>
#include <asio2/mqtt/message.hpp>

namespace asio2::mqtt
{
	template<
		class Key,   // client id
		class Value, // subscribe data
		class Container = std::unordered_map<Key, Value>
	>
	class subscription_map
	{
	public:
		using key_type = std::pair<std::size_t, std::string_view>;

		struct hasher
		{
			inline std::size_t operator()(key_type const& pair) const noexcept
			{
				std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
					(const unsigned char*)(std::addressof(pair.first)), sizeof(std::size_t));
				return asio2::detail::fnv1a_hash<std::size_t>(v,
					(const unsigned char*)(pair.second.data()), pair.second.size());
			}
		};

		struct node
		{
			std::size_t        id;
			std::size_t        count = 1;
			bool               has_plus = false;
			bool               has_hash = false;
			key_type           parent;
			std::vector<char>  tokenize; // vector has no SSO
			Container          subscribers;

			inline std::string_view tokenize_view()
			{
				return std::string_view{ tokenize.data(), tokenize.size() };
			}

			explicit node(std::size_t i, key_type p, std::string_view t) : id(i), parent(p)
			{
				tokenize.resize(t.size());
				std::memcpy((void*)tokenize.data(), (const void*)t.data(), t.size());
			}
		};

		using map_type = std::unordered_map<key_type, node, hasher>;
		using map_iterator       = typename map_type::iterator;
		using map_const_iterator = typename map_type::const_iterator;

	public:
		subscription_map()
		{
			this->root_node_id_ = idmgr_.get();
			ASIO2_ASSERT(this->root_node_id_ == 1);
			map_.emplace(root_key_, node(this->root_node_id_, root_key_, ""));
		}

		// Return the number of registered topic filters
		inline std::size_t get_subscribe_count() const
		{
			return this->subscribe_count_;
		}

		template<typename Function>
		void match(std::string_view topic, Function&& callback)
		{
			std::vector<map_iterator> iters;

			asio2::shared_locker g(this->submap_mutex_);

			iters.emplace_back(this->get_root());

			topic_filter_tokenizer(topic, [this, &iters, &callback](std::string_view t) mutable
			{
				return this->match_subfun(iters, callback, t);
			});

			for (auto& it : iters)
			{
				for (auto&[k, v] : it->second.subscribers)
				{
					callback(k, v);
				}
			}
		}

		// Insert a key => value at the specified topic filter
		// returns the handle and true if key was inserted, false if key was updated
		template <typename K, typename V>
		std::pair<key_type, bool> insert_or_assign(std::string_view topic_filter, K&& key, V&& val)
		{
			asio2::unique_locker g(this->submap_mutex_);

			std::vector<map_iterator> iters = this->get_nodes_by_topic_filter(topic_filter);
			if (iters.empty())
			{
				iters = this->emplace(topic_filter);

				this->emplace_subscriber_node(key, iters.back()->first);

				iters.back()->second.subscribers.insert_or_assign(std::forward<K>(key), std::forward<V>(val));

				this->increase_subscribe_count();

				return std::pair(iters.back()->first, true);
			}
			else
			{
				auto& subscribers = iters.back()->second.subscribers;

				this->emplace_subscriber_node(key, iters.back()->first);

				auto[_1, inserted] = subscribers.insert_or_assign(std::forward<K>(key), std::forward<V>(val));

				asio2::ignore_unused(_1, inserted);

				if (inserted)
				{
					this->increase_subscriptions(iters);
					this->increase_subscribe_count();
				}

				return std::pair(iters.back()->first, inserted);
			}
		}

		// Insert a key => value with a handle to the topic filter
		// returns the handle and true if key was inserted, false if key was updated
		template <typename K, typename V>
		std::pair<key_type, bool> insert_or_assign(key_type const& h, K&& key, V&& val)
		{
			asio2::unique_locker g(this->submap_mutex_);

			auto it = this->map_.find(h);
			if (it == this->map_.end())
			{
				return std::pair(key_type(0, "null"), false);
			}

			auto& subscribers = it->second.subscribers;

			this->emplace_subscriber_node(key, it->first);

			auto[_1, inserted] = subscribers.insert_or_assign(std::forward<K>(key), std::forward<V>(val));

			asio2::ignore_unused(_1, inserted);

			if (inserted)
			{
				this->increase_subscriptions(h);
				this->increase_subscribe_count();
			}

			return std::pair(h, inserted);
		}

		// returns the number of removed elements
		std::size_t erase(key_type const& h, Key const& key)
		{
			asio2::unique_locker g(this->submap_mutex_);

			auto it = this->map_.find(h);
			if (it == this->map_.end())
			{
				return 0;
			}

			this->erase_subscriber_node(key, h);

			auto amount = it->second.subscribers.erase(key);
			if (amount)
			{
				std::vector<map_iterator> v = this->handle_to_iterators(h);
				this->erase(v);
				this->decrease_subscribe_count();
			}

			return amount;
		}

		// returns the number of removed elements
		std::size_t erase(std::string_view topic_filter, Key const& key)
		{
			asio2::unique_locker g(this->submap_mutex_);

			std::vector<map_iterator> iters = this->get_nodes_by_topic_filter(topic_filter);
			if (iters.empty())
			{
				return 0;
			}

			this->erase_subscriber_node(key, iters.back()->first);

			auto amount = iters.back()->second.subscribers.erase(key);
			if (amount)
			{
				this->decrease_subscribe_count();
				this->erase(iters);
			}

			return amount;
		}

		// returns the number of removed elements
		std::size_t erase(Key const& key)
		{
			asio2::unique_locker g(this->submap_mutex_);

			auto iter = this->subscriber_nodes_.find(key);
			if (iter == this->subscriber_nodes_.end())
				return 0;

			std::size_t total = 0;

			for (auto& h : iter->second)
			{
				auto it = this->map_.find(h);
				if (it == this->map_.end())
					continue;

				auto amount = it->second.subscribers.erase(key);
				if (amount)
				{
					std::vector<map_iterator> v = this->handle_to_iterators(h);
					this->erase(v);
					this->decrease_subscribe_count();
				}

				total += amount;
			}

			this->subscriber_nodes_.erase(key);

			return total;
		}

	protected:
		inline map_iterator       get_root()       ASIO2_NO_THREAD_SAFETY_ANALYSIS { return map_.find(root_key_); };
		inline map_const_iterator get_root() const ASIO2_NO_THREAD_SAFETY_ANALYSIS { return map_.find(root_key_); };

		// Increase the map size (total number of subscriptions stored)
		inline void increase_subscribe_count()
		{
			++subscribe_count_;
		}

		// Decrease the map size (total number of subscriptions stored)
		inline void decrease_subscribe_count()
		{
			ASIO2_ASSERT(subscribe_count_ > 0);
			--subscribe_count_;
		}

		inline void increase_subscriptions(key_type const& h)
		{
			handle_to_iterators(h, [](map_iterator it) mutable
			{
				it->second.count++;
			});
		}

		// Increase the number of subscriptions for this path
		inline void increase_subscriptions(std::vector<map_iterator>& iters)
		{
			for (auto i : iters)
			{
				i->second.count++;
			}
		}
		std::optional<key_type> lookup(std::string_view topic_filter)
		{
			std::vector<map_iterator> iters = this->get_nodes_by_topic_filter(topic_filter);

			if (iters.empty())
				return std::nullopt;
			else
				return iters.back()->first;
		}

		std::vector<map_iterator> emplace(std::string_view topic_filter) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			map_iterator parent = this->get_root();

			std::vector<map_iterator> iters;

			topic_filter_tokenizer(topic_filter, [this, &iters, &parent](std::string_view t) mutable
			{
				return emplace_subfun(iters, parent, t);
			});

			return iters;
		}

		inline bool emplace_subfun(std::vector<map_iterator>& iters, map_iterator& parent, std::string_view t)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			node& pn = parent->second;

			auto it = map_.find(key_type(pn.id, t));

			if (it == map_.end())
			{
				node     val{ idmgr_.get(), parent->first, t };
				key_type key{ pn.id, val.tokenize_view() };

				it = map_.emplace(std::move(key), std::move(val)).first;

				pn.has_plus |= (t == "+");
				pn.has_hash |= (t == "#");
			}
			else
			{
				it->second.count++;
			}

			iters.emplace_back(it);
			parent = std::move(it);
			return true;
		}

		void erase(std::vector<map_iterator>& iters) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			bool remove_plus_flag = false;
			bool remove_hash_flag = false;

			std::reverse(iters.begin(), iters.end());

			for (map_iterator& it : iters)
			{
				node& n = it->second;

				if (remove_plus_flag)
				{
					n.has_plus = false;
					remove_plus_flag = false;
				}

				if (remove_hash_flag)
				{
					n.has_hash = false;
					remove_hash_flag = false;
				}

				ASIO2_ASSERT(n.count > 0);

				n.count--;

				if (n.count == 0)
				{
					remove_plus_flag = (it->first.second == "+");
					remove_hash_flag = (it->first.second == "#");

					this->idmgr_.release(it->second.id);

					// std::unordered_map<Key,T,Hash,KeyEqual,Allocator>::erase
					// References and iterators to the erased elements are invalidated.
					// Other iterators and references are not invalidated.
					map_.erase(it);
				}
			}

			map_iterator root = this->get_root();

			if (remove_plus_flag)
			{
				root->second.has_plus = false;
			}

			if (remove_hash_flag)
			{
				root->second.has_hash = false;
			}
		}

		template <typename K>
		inline void emplace_subscriber_node(K&& key, key_type node_key) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::unordered_set<key_type, hasher>& node_keys = this->subscriber_nodes_[key];
			node_keys.emplace(std::move(node_key));
		}

		inline void erase_subscriber_node(const Key& key, const key_type& node_key) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::unordered_set<key_type, hasher>& node_keys = this->subscriber_nodes_[key];
			node_keys.erase(node_key);
			if (node_keys.empty())
			{
				this->subscriber_nodes_.erase(key);
			}
		}

		std::vector<map_iterator> get_nodes_by_topic_filter(std::string_view topic_filter)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::size_t id = this->get_root()->second.id;

			std::vector<map_iterator> iters;

			topic_filter_tokenizer(topic_filter, [this, &iters, &id](std::string_view t) mutable
			{
				return this->get_nodes_by_topic_filter_subfun(iters, id, t);
			});

			return iters;
		}

		inline bool get_nodes_by_topic_filter_subfun(
			std::vector<map_iterator>& iters, std::size_t& id, std::string_view t) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			auto it = map_.find(key_type(id, t));

			if (it == map_.end())
			{
				iters.clear();
				return false;
			}

			id = it->second.id;
			iters.emplace_back(it);
			return true;
		}

		template<typename Function>
		bool match_subfun(std::vector<map_iterator>& iters, Function& callback, std::string_view t)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::vector<map_iterator> new_iters;

			for (auto& it : iters)
			{
				node& pn = it->second;

				auto i = this->map_.find(key_type(pn.id, t));
				if (i != this->map_.end())
				{
					new_iters.emplace_back(i);
				}

				if (pn.has_plus)
				{
					i = this->map_.find(key_type(pn.id, std::string_view("+")));
					if (i != this->map_.end())
					{
						if (pn.id != this->root_node_id_ || t.empty() || t[0] != '$')
						{
							new_iters.emplace_back(i);
						}
					}
				}

				if (pn.has_hash)
				{
					i = this->map_.find(key_type(pn.id, std::string_view("#")));
					if (i != this->map_.end())
					{
						if (pn.id != this->root_node_id_ || t.empty() || t[0] != '$')
						{
							for (auto& [k, v] : i->second.subscribers)
							{
								callback(k, v);
							}
						}
					}
				}
			}

			std::swap(iters, new_iters);

			return !iters.empty();
		}

		template<typename Function>
		void handle_to_iterators(key_type const& h, Function&& callback) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			key_type k = h;
			while (k != this->root_key_)
			{
				auto it = this->map_.find(k);
				if (it == this->map_.end())
				{
					return;
				}

				callback(it);

				k = it->second.parent;
			}
		}

		inline std::vector<map_iterator> handle_to_iterators(key_type const& h) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::vector<map_iterator> iters;
			this->handle_to_iterators(h, [&iters](map_iterator it) mutable
			{
				iters.emplace_back(it);
			});
			std::reverse(iters.begin(), iters.end());
			return iters;
		}

		// Get path of topic_filter
		std::string handle_to_topic_filter(key_type const& h) const ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::string result;

			handle_to_iterators(h, [&result](map_iterator it) mutable
			{
				if (result.empty())
				{
					result = std::string(it->first.second);
				}
				else
				{
					result.insert(0, "/");
					result.insert(0, it->first.second);
				}
			});

			return result;
		}

	protected:
		static constexpr key_type                                     root_key_{ 0, "" };

		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer                                 submap_mutex_;

		std::size_t                                                   root_node_id_ = 1;

		map_type                                                      map_              ASIO2_GUARDED_BY(submap_mutex_);

		// Key - client id, Val - all nodes keys for the subscriber
		std::unordered_map<Key, std::unordered_set<key_type, hasher>> subscriber_nodes_ ASIO2_GUARDED_BY(submap_mutex_);

		// Map size tracks the total number of subscriptions within the map
		std::atomic<std::size_t>                                      subscribe_count_ = 0;

		mqtt::idmgr<std::set<std::size_t>>                            idmgr_;
	};
}

#endif // !__ASIO2_MQTT_SUBSCRIPTION_MAP_HPP__
