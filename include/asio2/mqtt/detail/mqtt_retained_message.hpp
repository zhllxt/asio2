/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/mqtt/mqtt_protocol_v3.hpp>
#include <asio2/mqtt/mqtt_protocol_v4.hpp>
#include <asio2/mqtt/mqtt_protocol_v5.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	template<typename Value>
	class retained_messages
	{
	public:
		using path_entry_key = std::pair<std::size_t, std::string_view>;

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

		using map_type = std::unordered_map<path_entry_key, path_entry, std::hash<path_entry_key>>;
		using map_type_iterator       = typename map_type::iterator;
		using map_type_const_iterator = typename map_type::const_iterator;

		std::unordered_map<path_entry_key, path_entry, std::hash<path_entry_key>> map;
		std::unordered_multimap<std::size_t, path_entry*> wildcard_map;

		std::size_t map_size;
		std::size_t next_node_id;

		inline map_type_iterator create_topic(std::string_view topic_name)
		{
			map_type_iterator parent = get_root();

			topic_filter_tokenizer(topic_name, [this, &parent](std::string_view t) mutable
			{
				if (t == "+" || t == "#")
				{
					throw_no_wildcards_allowed();
				}

				std::size_t parent_id = parent->second.id;

				map_type_iterator entry = map.find(path_entry_key(parent_id, t));

				if (entry == map.end())
				{
					entry = map.emplace(
						path_entry_key(parent_id, t),
						path_entry(parent_id, t, next_node_id++)
					).first;

					wildcard_map.emplace(parent_id, &(entry->second));

					if (next_node_id == max_node_id)
					{
						throw_max_stored_topics();
					}
				}
				else
				{
					entry->second.increase_count();
				}

				parent = entry;
				return true;
			});

			return parent;
		}

		inline std::vector<map_type_iterator> find_topic(std::string_view topic_name)
		{
			std::vector<map_type_iterator> path;

			map_type_iterator parent = get_root();

			topic_filter_tokenizer(topic_name, [this, &parent, &path](std::string_view t) mutable
			{
				auto entry = map.find(path_entry_key(parent->second.id, t));

				if (entry == map.end())
				{
					path.clear();
					return false;
				}

				path.push_back(entry);
				parent = entry;
				return true;
			});

			return path;
		}

		// Match all underlying topics when a hash entry is matched
		// perform a breadth-first iteration over all items in the tree below
		template<typename Output>
		inline void match_hash_entries(std::size_t parent_id, Output&& callback, bool ignore_system)
		{
			std::deque<std::size_t> entries;
			entries.push_back(parent_id);

			std::deque<std::size_t> new_entries;

			while (!entries.empty())
			{
				new_entries.resize(0);

				for (auto entry : entries)
				{
					// Find all entries below this node
					auto range = wildcard_map.equal_range(entry);
					for (auto i = range.first; i != range.second && i->second->parent_id == entry; ++i)
					{
						// Should we ignore system matches
						if (!ignore_system || i->second->name.empty() || i->second->name[0] != '$')
						{
							if (i->second->value)
							{
								callback(i->second->value.value());
							}

							new_entries.push_back(i->second->id);
						}
					}
				}

				// Ignore system only on first level
				ignore_system = false;

				std::swap(entries, new_entries);
			}
		}

		// Find all topics that match the specified topic filter
		template<typename Output>
		inline void find_match(std::string_view topic_filter, Output&& callback)
		{
			std::deque<map_type_iterator> entries;
			entries.push_back(get_root());

			std::deque<map_type_iterator> new_entries;

			topic_filter_tokenizer(topic_filter,
			[this, &entries, &new_entries, &callback](std::string_view t) mutable
			{
				new_entries.resize(0);

				for (auto& entry : entries)
				{
					std::size_t parent_id = entry->second.id;

					if (t == std::string_view("+"))
					{
						auto range = wildcard_map.equal_range(parent_id);

						for (auto i = range.first; i != range.second && i->second->parent_id == parent_id; ++i)
						{
							if (parent_id != root_node_id || i->second->name.empty() || i->second->name[0] != '$')
							{
								auto it = map.find(path_entry_key(i->second->parent_id, i->second->name));
								ASIO2_ASSERT(it != map.end());
								new_entries.push_back(it);
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
						map_type_iterator i = map.find(path_entry_key(parent_id, t));
						if (i != map.end())
						{
							new_entries.push_back(i);
						}
					}
				}

				std::swap(new_entries, entries);

				return !entries.empty();
			});

			for (auto& entry : entries)
			{
				if (entry->second.value)
				{
					callback(entry->second.value.value());
				}
			}
		}

		// Remove a value at the specified topic name
		inline std::size_t erase_topic(std::string_view topic_name)
		{
			auto path = find_topic(topic_name);

			// Reset the value if there is actually something stored
			if (!path.empty() && path.back()->second.value)
			{
				path.back()->second.value = std::nullopt;

				// Do iterators stay valid when erasing ? I think they do ?
				for (auto entry : path)
				{
					entry->second.decrease_count();

					if (entry->second.count == 0)
					{
						auto range = wildcard_map.equal_range(std::get<0>(entry->first));

						for (auto it = range.first; it != range.second; ++it)
						{
							if (&(entry->second) == it->second)
							{
								wildcard_map.erase(it);
								break;
							}
						}

						map.erase(entry);
					}
				}

				return 1;
			}

			return 0;
		}

		// Increase the number of topics for this path
		inline void increase_topics(std::vector<map_type_iterator> const &path)
		{
			for (auto& entry : path)
			{
				entry->second.increase_count();
			}
		}

		// Increase the map size (total number of topics stored)
		inline void increase_map_size()
		{
			if (map_size == (std::numeric_limits<decltype(map_size)>::max)())
			{
				throw_max_stored_topics();
			}

			++map_size;
		}

		// Decrease the map size (total number of topics stored)
		inline void decrease_map_size(std::size_t count)
		{
			ASIO2_ASSERT(map_size >= count);
			map_size -= count;
		}

		inline void init_map()
		{
			map_size = 0;
			// Create the root node
			auto entry = map.emplace(path_entry_key(root_parent_id, ""),
				path_entry(root_parent_id, "", root_node_id)).first;
			next_node_id = root_node_id + 1;
			// 
			wildcard_map.emplace(root_parent_id, &(entry->second));
		}

		inline map_type_iterator get_root()
		{
			return map.find(path_entry_key(root_parent_id, ""));
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
			find_match(topic_filter, std::forward<Output>(callback));
		}

		// Remove a stored value at the specified topic name
		inline std::size_t erase(std::string_view topic_name)
		{
			auto result = erase_topic(topic_name);
			decrease_map_size(result);
			return result;
		}

		// Get the number of entries stored in the map
		inline std::size_t size() const { return map_size; }

		// Get the number of entries in the map (for debugging purpose only)
		inline std::size_t internal_size() const { return map.size(); }

		// Clear all topics
		inline void clear()
		{
			map.clear();
			wildcard_map.clear();
			init_map();
		}

		// Dump debug information
		template<typename Output>
		inline void dump(Output &out)
		{
			for (auto const&[k, v] : map)
			{
				std::ignore = k;
				out << v.parent_id << " " << v.name << " " << (v.value ? "init" : "-")
					<< " " << v.count << std::endl;
			}
		}
	};

	// A collection of messages that have been retained in
	// case clients add a new subscription to the associated topics.
	struct retained_entry
	{
		template<class Message>
		explicit retained_entry(Message&& msg, std::shared_ptr<asio::steady_timer> expiry_timer)
			: message(std::forward<Message>(msg))
			, message_expiry_timer(std::move(expiry_timer))
		{
		}

		std::variant<v3::publish, v4::publish, v5::publish> message;
		std::shared_ptr<asio::steady_timer> message_expiry_timer;
	};
}

#endif // !__ASIO2_MQTT_RETAINED_MESSAGES_HPP__
