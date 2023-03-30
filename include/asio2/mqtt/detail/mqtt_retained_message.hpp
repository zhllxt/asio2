/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/retained_messages.hpp
 */

#ifndef __ASIO2_MQTT_RETAINED_MESSAGES_HPP__
#define __ASIO2_MQTT_RETAINED_MESSAGES_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <deque>

#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/message.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	template<typename Value>
	class retained_messages
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

	protected:
		// Exceptions used
		static void throw_max_stored_topics()
		{
			throw std::overflow_error("Retained map maximum number of topics reached");
		}
		static void throw_no_wildcards_allowed()
		{
			throw std::runtime_error("Retained map no wildcards allowed in retained topic name");
		}

		static constexpr std::size_t root_parent_id = 0;
		static constexpr std::size_t root_node_id   = 1;
		static constexpr std::size_t max_node_id    = (std::numeric_limits<std::size_t>::max)();

		struct path_entry
		{
			std::size_t      parent_id;
			std::string_view name;
			std::size_t      id;
			std::size_t      count = 1;

			static constexpr std::size_t max_count = (std::numeric_limits<std::size_t>::max)();

			// Increase the count for this node
			inline void increase_count()
			{
				if (count == max_count)
				{
					throw_max_stored_topics();
				}

				++count;
			}

			// Decrease the count for this node
			inline void decrease_count()
			{
				ASIO2_ASSERT(count >= 1);
				--count;
			}

			std::optional<Value> value;

			path_entry(std::size_t parent_id, std::string_view name, std::size_t id)
				: parent_id(parent_id)
				, name(name)
				, id(id)
			{ }
		};

		using map_type = std::unordered_map<key_type, path_entry, hasher>;
		using map_iterator       = typename map_type::iterator;
		using map_const_iterator = typename map_type::const_iterator;

		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer  retained_mutex_;

		std::unordered_map     <key_type, path_entry, hasher> map_          ASIO2_GUARDED_BY(retained_mutex_);
		std::unordered_multimap<std::size_t, path_entry*    > wildcard_map_ ASIO2_GUARDED_BY(retained_mutex_);

		std::size_t map_size     ASIO2_GUARDED_BY(retained_mutex_);
		std::size_t next_node_id ASIO2_GUARDED_BY(retained_mutex_);

		inline map_iterator create_topic(std::string_view topic_name) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			map_iterator parent = get_root();

			topic_filter_tokenizer(topic_name, [this, &parent](std::string_view t) mutable
			{
				return this->create_topic_subfun(parent, t);
			});

			return parent;
		}

		inline bool create_topic_subfun(map_iterator& parent, std::string_view t) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			if (t == "+" || t == "#")
			{
				throw_no_wildcards_allowed();
			}

			std::size_t parent_id = parent->second.id;

			map_iterator it = map_.find(key_type(parent_id, t));

			if (it == map_.end())
			{
				it = map_.emplace(
					key_type(parent_id, t),
					path_entry(parent_id, t, next_node_id++)
				).first;

				wildcard_map_.emplace(parent_id, std::addressof(it->second));

				if (next_node_id == max_node_id)
				{
					throw_max_stored_topics();
				}
			}
			else
			{
				it->second.increase_count();
			}

			parent = it;
			return true;
		}

		inline std::vector<map_iterator> find_topic(std::string_view topic_name) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::vector<map_iterator> path;

			map_iterator parent = get_root();

			topic_filter_tokenizer(topic_name, [this, &path, &parent](std::string_view t) mutable
			{
				return this->find_topic_subfun(path, parent, t);
			});

			return path;
		}

		inline bool find_topic_subfun(std::vector<map_iterator>& path, map_iterator& parent, std::string_view t)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			auto it = map_.find(key_type(parent->second.id, t));

			if (it == map_.end())
			{
				path.clear();
				return false;
			}

			path.push_back(it);
			parent = it;
			return true;
		}

		// Match all underlying topics when a hash entry is matched
		// perform a breadth-first iteration over all items in the tree below
		template<typename Output>
		inline void match_hash_entries(std::size_t parent_id, Output&& callback, bool ignore_system)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::deque<std::size_t> ids;
			ids.push_back(parent_id);

			std::deque<std::size_t> new_ids;

			while (!ids.empty())
			{
				new_ids.resize(0);

				for (auto it : ids)
				{
					auto range = wildcard_map_.equal_range(it);
					for (auto i = range.first; i != range.second && i->second->parent_id == it; ++i)
					{
						// Should we ignore system matches
						if (!ignore_system || i->second->name.empty() || i->second->name[0] != '$')
						{
							if (i->second->value)
							{
								callback(i->second->value.value());
							}

							new_ids.push_back(i->second->id);
						}
					}
				}

				// Ignore system only on first level
				ignore_system = false;

				std::swap(ids, new_ids);
			}
		}

		// Find all topics that match the specified topic filter
		template<typename Output>
		inline void find_match(std::string_view topic_filter, Output&& callback) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			std::deque<map_iterator> iters;
			iters.push_back(get_root());

			std::deque<map_iterator> new_iters;

			topic_filter_tokenizer(topic_filter,
			[this, &iters, &new_iters, &callback](std::string_view t) mutable
			{
				return this->find_match_subfun(iters, new_iters, callback, t);
			});

			for (auto& it : iters)
			{
				if (it->second.value)
				{
					callback(it->second.value.value());
				}
			}
		}

		template<typename Output>
		inline bool find_match_subfun(
			std::deque<map_iterator>& iters, std::deque<map_iterator>& new_iters, Output& callback, std::string_view t)
			ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			new_iters.resize(0);

			for (auto& it : iters)
			{
				std::size_t parent_id = it->second.id;

				if (t == std::string_view("+"))
				{
					auto range = wildcard_map_.equal_range(parent_id);

					for (auto i = range.first; i != range.second && i->second->parent_id == parent_id; ++i)
					{
						if (parent_id != root_node_id || i->second->name.empty() || i->second->name[0] != '$')
						{
							auto j = map_.find(key_type(i->second->parent_id, i->second->name));
							ASIO2_ASSERT(j != map_.end());
							new_iters.push_back(j);
						}
						else
						{
							break;
						}
					}
				}
				else if (t == std::string_view("#"))
				{
					match_hash_entries(parent_id, callback, parent_id == root_node_id);
					return false;
				}
				else
				{
					map_iterator i = map_.find(key_type(parent_id, t));
					if (i != map_.end())
					{
						new_iters.push_back(i);
					}
				}
			}

			std::swap(new_iters, iters);

			return !iters.empty();
		}

		// Remove a value at the specified topic name
		inline std::size_t erase_topic(std::string_view topic_name) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			auto path = find_topic(topic_name);

			// Reset the value if there is actually something stored
			if (!path.empty() && path.back()->second.value)
			{
				path.back()->second.value = std::nullopt;

				// Do iterators stay valid when erasing ? I think they do ?
				for (auto iter : path)
				{
					iter->second.decrease_count();

					if (iter->second.count == 0)
					{
						auto range = wildcard_map_.equal_range(std::get<0>(iter->first));

						for (auto it = range.first; it != range.second; ++it)
						{
							if (std::addressof(iter->second) == it->second)
							{
								wildcard_map_.erase(it);
								break;
							}
						}

						map_.erase(iter);
					}
				}

				return 1;
			}

			return 0;
		}

		// Increase the number of topics for this path
		inline void increase_topics(std::vector<map_iterator> const &path) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			for (auto& it : path)
			{
				it->second.increase_count();
			}
		}

		// Increase the map size (total number of topics stored)
		inline void increase_map_size() ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			if (map_size == (std::numeric_limits<decltype(map_size)>::max)())
			{
				throw_max_stored_topics();
			}

			++map_size;
		}

		// Decrease the map size (total number of topics stored)
		inline void decrease_map_size(std::size_t count) ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			ASIO2_ASSERT(map_size >= count);
			map_size -= count;
		}

		inline void init_map() ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			map_size = 0;
			// Create the root node
			auto it = map_.emplace(key_type(root_parent_id, ""),
				path_entry(root_parent_id, "", root_node_id)).first;
			next_node_id = root_node_id + 1;
			// 
			wildcard_map_.emplace(root_parent_id, std::addressof(it->second));
		}

		inline map_iterator get_root() ASIO2_NO_THREAD_SAFETY_ANALYSIS
		{
			return map_.find(key_type(root_parent_id, ""));
		}

	public:
		retained_messages()
		{
			init_map();
		}

		// Insert a value at the specified topic name
		template<typename V>
		inline std::size_t insert_or_assign(std::string_view topic_name, V&& value)
		{
			asio2::unique_locker g(this->retained_mutex_);

			auto path = this->find_topic(topic_name);

			if (path.empty())
			{
				auto new_topic = this->create_topic(topic_name);
				new_topic->second.value.emplace(std::forward<V>(value));
				increase_map_size();
				return 1;
			}

			if (!path.back()->second.value)
			{
				this->increase_topics(path);
				path.back()->second.value.emplace(std::forward<V>(value));
				increase_map_size();
				return 1;
			}

			// replace the value
			path.back()->second.value.emplace(std::forward<V>(value));

			return 0;
		}

		// Find all stored topics that math the specified topic_filter
		template<typename Output>
		inline void find(std::string_view topic_filter, Output&& callback)
		{
			asio2::shared_locker g(this->retained_mutex_);

			find_match(topic_filter, std::forward<Output>(callback));
		}

		// Remove a stored value at the specified topic name
		inline std::size_t erase(std::string_view topic_name)
		{
			asio2::unique_locker g(this->retained_mutex_);

			auto result = erase_topic(topic_name);

			decrease_map_size(result);

			return result;
		}

		inline std::size_t size() const
		{
			asio2::shared_locker g(this->retained_mutex_);

			return map_size;
		}

		inline std::size_t internal_size() const
		{
			asio2::shared_locker g(this->retained_mutex_);

			return map_.size();
		}

		// Clear all topics
		inline void clear()
		{
			asio2::unique_locker g(this->retained_mutex_);

			map_.clear();
			wildcard_map_.clear();
			init_map();
		}

		// Dump debug information
		template<typename Output>
		inline void dump(Output &out)
		{
			asio2::shared_locker g(this->retained_mutex_);

			for (auto const&[k, v] : map_)
			{
				std::ignore = k;
				out << v.parent_id << " " << v.name << " " << (v.value ? "init" : "-")
					<< " " << v.count << std::endl;
			}
		}
	};

	// A collection of messages that have been retained in
	// case clients add a new subscription to the associated topics.
	struct rmnode
	{
		template<class Message>
		explicit rmnode(Message&& msg, std::shared_ptr<asio::steady_timer> expiry_timer)
			: message(std::forward<Message>(msg))
			, message_expiry_timer(std::move(expiry_timer))
		{
		}

		mqtt::message message;
		std::shared_ptr<asio::steady_timer> message_expiry_timer;
	};
}

#endif // !__ASIO2_MQTT_RETAINED_MESSAGES_HPP__
