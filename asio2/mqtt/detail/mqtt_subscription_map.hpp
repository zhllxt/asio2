/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <vector>

#include <asio2/mqtt/mqtt_protocol_util.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	// Combined storage for count and flags
	// we can have 32bit or 64bit version

	// Compile error on other platforms (not 32 or 64 bit)
	template<std::size_t N>
	struct count_storage
	{
		static_assert(N == 4 || N == 8, "Subscription map count_storage only knows how to handle architectures with 32 or 64 bit size_t: please update to support your platform.");
	};

	template<>
	struct count_storage<4>
	{
	public:
		count_storage(std::uint32_t v = 1)
			: value_(v & 0x3fffffffUL), has_hash_child_(false), has_plus_child_(false)
		{ }

		static constexpr std::size_t max() { return (std::numeric_limits<std::uint32_t>::max)() >> 2; }

		inline std::uint32_t value() const { return value_; }
		inline void set_value(std::uint32_t v) { value_ = v & 0x3fffffffUL; }
		inline void increment_value() { ++value_; }
		inline void decrement_value() { --value_; }

		inline bool has_hash_child() const { return has_hash_child_; }
		inline void set_hash_child(bool v) { has_hash_child_ = v; }

		inline bool has_plus_child() const { return has_plus_child_; }
		inline void set_plus_child(bool v) { has_plus_child_ = v; }

	protected:
		std::uint32_t value_ : 30;
		std::uint32_t has_hash_child_ : 1;
		std::uint32_t has_plus_child_ : 1;
	};

	template<>
	struct count_storage<8>
	{
	public:
		count_storage(std::uint64_t v = 1)
			: value_(v & 0x3fffffffffffffffULL), has_hash_child_(false), has_plus_child_(false)
		{ }

		static constexpr std::uint64_t max() { return (std::numeric_limits<std::uint64_t>::max)() >> 2; }

		inline std::uint64_t value() const { return value_; }
		inline void set_value(std::uint64_t v) { value_ = v & 0x3fffffffffffffffULL; }
		inline void increment_value() { ++value_; }
		inline void decrement_value() { --value_; }

		inline bool has_hash_child() const { return has_hash_child_; }
		inline void set_hash_child(bool v) { has_hash_child_ = v; }

		inline bool has_plus_child() const { return has_plus_child_; }
		inline void set_plus_child(bool v) { has_plus_child_ = v; }

	protected:
		std::uint64_t value_ : 62;
		std::uint64_t has_hash_child_ : 1;
		std::uint64_t has_plus_child_ : 1;
	};

	template<typename Value>
	class subscription_map_base
	{
	public:
		using path_entry_key  = std::pair<std::size_t, std::string_view>;
		using handle          = path_entry_key;
		using count_storage_t = count_storage<sizeof(void *)>;

	protected:
		// Generate a node id for a new node
		inline std::size_t generate_node_id()
		{
			if (next_node_id == (std::numeric_limits<std::size_t>::max)())
				throw_max_stored_topics();
			return ++next_node_id;
		}

		struct path_entry
		{
			std::size_t        id;
			path_entry_key     parent;
			count_storage_t    count;
			Value              value;
			std::vector<char>  string; // vector has no SSO

			inline std::string_view key_view()
			{
				return std::string_view{ string.data(), string.size() };
			}

			explicit path_entry(std::size_t id, path_entry_key parent, std::string_view str)
				: id(id), parent(parent)
			{
				string.resize(str.size());
				std::memcpy((void*)string.data(), (const void*)str.data(), str.size());
			}
		};

		// Increase the subscription count for a specific node
		static void increase_count_storage(count_storage_t &count)
		{
			if (count.value() == (count_storage_t::max)())
			{
				throw_max_stored_topics();
			}

			count.increment_value();
		}

		// Decrease the subscription count for a specific node
		static void decrease_count_storage(count_storage_t& count)
		{
			ASIO2_ASSERT(count.value() > 0);
			count.decrement_value();
		}

		using this_type = subscription_map_base<Value>;

		using map_type = std::unordered_map<path_entry_key, path_entry, std::hash<path_entry_key>>;
		using map_type_iterator       = typename map_type::iterator;
		using map_type_const_iterator = typename map_type::const_iterator;

		map_type map;

		std::size_t next_node_id = 0;

	protected:
	public:
		// Key and id of the root key
		path_entry_key root_key;
		std::size_t root_node_id;

		// Return the iterator of the root
		inline map_type_iterator get_root() { return map.find(root_key); };
		inline map_type_const_iterator get_root() const { return map.find(root_key); };

		// Map size tracks the total number of subscriptions within the map
		size_t map_size = 0;

		inline map_type_iterator get_key(path_entry_key key) { return map.find(key); }
		inline map_type_iterator begin() { return map.begin(); }
		inline map_type_iterator end() { return map.end(); }
		inline map_type const& get_map() const { return map; }

		inline handle path_to_handle(std::vector<map_type_iterator> const& path) const
		{
			return path.back()->first;
		}

		inline std::vector<map_type_iterator> find_topic_filter(std::string_view topic_filter)
		{
			auto parent_id = this->get_root()->second.id;
			std::vector<map_type_iterator> path;

			topic_filter_tokenizer(topic_filter,
			[this, &path, &parent_id](std::string_view t) mutable
			{
				auto entry = map.find(path_entry_key(parent_id, t));

				if (entry == map.end())
				{
					path.clear();
					return false;
				}

				path.push_back(entry);
				parent_id = entry->second.id;
				return true;
			});

			return path;
		}

		inline std::vector<map_type_iterator> create_topic_filter(std::string_view topic_filter)
		{
			auto parent = get_root();

			std::vector<map_type_iterator> result;

			topic_filter_tokenizer(topic_filter,
			[this, &parent, &result](std::string_view t) mutable
			{
				auto entry = map.find(path_entry_key(parent->second.id, t));

				if (entry == map.end())
				{
					path_entry     val{ generate_node_id(), parent->first, t };
					path_entry_key key{ parent->second.id, val.key_view() };

					entry = map.emplace(std::move(key), std::move(val)).first;

					parent->second.count.set_plus_child(parent->second.count.has_plus_child() | (t == "+"));
					parent->second.count.set_hash_child(parent->second.count.has_hash_child() | (t == "#"));
				}
				else
				{
					increase_count_storage(entry->second.count);
				}

				result.push_back(entry);
				parent = entry;
				return true;
			});

			return result;
		}

		// Remove a value at the specified path
		inline void remove_topic_filter(std::vector<map_type_iterator>& path)
		{
			bool remove_plus_child_flag = false;
			bool remove_hash_child_flag = false;

			std::reverse(path.begin(), path.end());

			// Go through entries to remove
			for (auto& entry : path)
			{
				if (remove_plus_child_flag)
				{
					entry->second.count.set_plus_child(false);
					remove_plus_child_flag = false;
				}

				if (remove_hash_child_flag)
				{
					entry->second.count.set_hash_child(false);
					remove_hash_child_flag = false;
				}

				decrease_count_storage(entry->second.count);

				if (entry->second.count.value() == 0)
				{
					remove_plus_child_flag = (entry->first.second == "+");
					remove_hash_child_flag = (entry->first.second == "#");

					// Erase in unordered map only invalidates erased iterator
					// other iterators are unaffected
					map.erase(entry->first);
				}
			}

			auto root = get_root();

			if (remove_plus_child_flag)
			{
				root->second.count.set_plus_child(false);
			}

			if (remove_hash_child_flag)
			{
				root->second.count.set_hash_child(false);
			}
		}

		template <typename ThisType, typename Output>
		static void find_match_impl(ThisType& self, std::string_view topic, Output&& callback)
		{
			// const_iterator or iterator depends on self
			using iterator_type = decltype(self.map.end());

			std::vector<iterator_type> entries;
			entries.push_back(self.get_root());

			topic_filter_tokenizer(topic, [&self, &entries, &callback](std::string_view t) mutable
			{
				std::vector<iterator_type> new_entries;

				for (auto& entry : entries)
				{
					auto parent = entry->second.id;
					auto i = self.map.find(path_entry_key(parent, t));
					if (i != self.map.end())
					{
						new_entries.push_back(i);
					}

					if (entry->second.count.has_plus_child())
					{
						i = self.map.find(path_entry_key(parent, std::string_view("+")));
						if (i != self.map.end())
						{
							if (parent != self.root_node_id || t.empty() || t[0] != '$')
							{
								new_entries.push_back(i);
							}
						}
					}

					if (entry->second.count.has_hash_child())
					{
						i = self.map.find(path_entry_key(parent, std::string_view("#")));
						if (i != self.map.end())
						{
							if (parent != self.root_node_id || t.empty() || t[0] != '$')
							{
								callback(i->second.value);
							}
						}
					}
				}

				std::swap(entries, new_entries);
				return !entries.empty();
			});

			for (auto& entry : entries)
			{
				callback(entry->second.value);
			}
		}

		// Find all topic filters that match the specified topic
		template<typename Output>
		inline void find_match(std::string_view topic, Output&& callback) const
		{
			find_match_impl(*this, topic, std::forward<Output>(callback));
		}

		// Find all topic filters and allow modification
		template<typename Output>
		inline void modify_match(std::string_view topic, Output&& callback)
		{
			find_match_impl(*this, topic, std::forward<Output>(callback));
		}

		template<typename ThisType, typename Output>
		static void handle_to_iterators(ThisType& self, handle const &h, Output&& output)
		{
			auto i = h;
			while (i != self.root_key)
			{
				auto entry_iter = self.map.find(i);
				if (entry_iter == self.map.end())
				{
					throw_invalid_handle();
				}

				output(entry_iter);
				i = entry_iter->second.parent;
			}
		}

		// Exceptions used
		static void throw_invalid_topic_filter()
		{
			throw std::runtime_error("Subscription map invalid topic filter was specified");
		}
		static void throw_invalid_handle()
		{
			throw std::runtime_error("Subscription map invalid handle was specified");
		}
		static void throw_max_stored_topics()
		{
			throw std::overflow_error("Subscription map maximum number of stored topic filters reached");
		}

		// Get the iterators of a handle
		inline std::vector<map_type_iterator> handle_to_iterators(handle const &h)
		{
			std::vector<map_type_iterator> result;
			handle_to_iterators(*this, h, [&result](map_type_iterator i) mutable { result.push_back(i); });
			std::reverse(result.begin(), result.end());
			return result;
		}

		// Increase the number of subscriptions for this handle
		inline void increase_subscriptions(handle const &h)
		{
			handle_to_iterators(*this, h, [](map_type_iterator i) mutable
			{
				increase_count_storage(i->second.count);
			});
		}

		// Increase the map size (total number of subscriptions stored)
		inline void increase_map_size()
		{
			if (map_size == (std::numeric_limits<decltype(map_size)>::max)())
			{
				throw_max_stored_topics();
			}

			++map_size;
		}

		// Decrease the map size (total number of subscriptions stored)
		inline void decrease_map_size()
		{
			ASIO2_ASSERT(map_size > 0);
			--map_size;
		}

		// Increase the number of subscriptions for this path
		inline void increase_subscriptions(std::vector<map_type_iterator> const &path)
		{
			for (auto i : path)
			{
				increase_count_storage(i->second.count);
			}
		}

		subscription_map_base()
		{
			// Create the root node
			root_node_id = generate_node_id();
			root_key = path_entry_key(generate_node_id(), std::string_view());
			map.emplace(root_key, path_entry(root_node_id, path_entry_key(), std::string_view()));
		}

	public:
		// Return the number of elements in the tree
		std::size_t internal_size() const { return map.size(); }

		// Return the number of registered topic filters
		std::size_t size() const { return this->map_size; }

		// Lookup a topic filter
		inline std::optional<handle> lookup(std::string_view topic_filter)
		{
			auto path = this->find_topic_filter(topic_filter);
			if (path.empty())
				return std::optional<handle>();
			else
				return this->path_to_handle(std::move(path));
		}

		// Get path of topic_filter
		inline std::string handle_to_topic_filter(handle const &h) const
		{
			std::string result;

			handle_to_iterators(*this, h, [&result](map_type_const_iterator i) mutable
			{
				if (result.empty())
				{
					result = std::string(i->first.second);
				}
				else
				{
					result = std::string(i->first.second) + "/" + result;
				}
			});

			return result;
		}
	};

	template<typename Value>
	class single_subscription_map : public subscription_map_base<std::optional<Value>>
	{
	public:
		// Handle of an entry
		using handle = typename subscription_map_base<Value>::handle;

		using map_type = typename subscription_map_base<Value>::map_type;
		using map_type_iterator       = typename subscription_map_base<Value>::map_type_iterator;
		using map_type_const_iterator = typename subscription_map_base<Value>::map_type_const_iterator;

		// Insert a value at the specified topic_filter
		template <typename V>
		std::pair<handle, bool> insert(std::string_view topic_filter, V&& value)
		{
			auto existing_subscription = this->find_topic_filter(topic_filter);
			if (!existing_subscription.empty())
			{
				if (existing_subscription.back()->second.value)
					return std::make_pair(this->path_to_handle(std::move(existing_subscription)), false);

				existing_subscription.back()->second.value.emplace(std::forward<V>(value));
				return std::make_pair(this->path_to_handle(std::move(existing_subscription)), true);
			}

			auto new_topic_filter = this->create_topic_filter(topic_filter);
			new_topic_filter.back()->second.value = value;
			this->increase_map_size();
			return std::make_pair(this->path_to_handle(std::move(new_topic_filter)), true);
		}

		// Update a value at the specified topic filter
		template <typename V>
		void update(std::string_view topic_filter, V&& value)
		{
			auto path = this->find_topic_filter(topic_filter);
			if (path.empty())
			{
				this->throw_invalid_topic_filter();
			}

			path.back()->second.value.emplace(std::forward<V>(value));
		}

		template <typename V>
		void update(handle const& h, V&& value)
		{
			auto entry_iter = this->get_key(h);
			if (entry_iter == this->end())
			{
				this->throw_invalid_topic_filter();
			}
			entry_iter->second.value.emplace(std::forward<V>(value));
		}

		// Remove a value at the specified topic filter
		std::size_t erase(std::string_view topic_filter)
		{
			auto path = this->find_topic_filter(topic_filter);
			if (path.empty() || !path.back()->second.value)
			{
				return 0;
			}

			this->remove_topic_filter(path);
			this->decrease_map_size();
			return 1;
		}

		// Remove a value using a handle
		std::size_t erase(handle const& h)
		{
			auto path = this->handle_to_iterators(h);
			if (path.empty() || !path.back()->second.value)
			{
				return 0;
			}

			this->remove_topic_filter(path);
			this->decrease_map_size();
			return 1;
		}

		// Find all topic filters that match the specified topic
		template<typename Output>
		void find(std::string_view topic, Output&& callback) const
		{
			this->find_match(topic, [&callback](std::optional<Value> const& value) mutable
			{
				if (value)
				{
					callback(value.value());
				}
			});
		}
	};

	template<
		typename Key,
		typename Value,
		class Hash = std::hash<Key>,
		class Pred = std::equal_to<Key>,
		class Container = std::unordered_map<Key, Value, Hash, Pred, std::allocator<std::pair<const Key, Value>>>
	>
	class multiple_subscription_map : public subscription_map_base<Container>
	{
	public:
		using container_t = Container;

		// Handle of an entry
		using handle = typename subscription_map_base<Value>::handle;

		using map_type = typename subscription_map_base<Container>::map_type;
		using map_type_iterator       = typename subscription_map_base<Container>::map_type_iterator;
		using map_type_const_iterator = typename subscription_map_base<Container>::map_type_const_iterator;

		// Insert a key => value at the specified topic filter
		// returns the handle and true if key was inserted, false if key was updated
		template <typename K, typename V>
		inline std::pair<handle, bool> insert_or_assign(std::string_view topic_filter, K&& key, V&& value)
		{
			auto path = this->find_topic_filter(topic_filter);
			if (path.empty())
			{
				auto new_topic_filter = this->create_topic_filter(topic_filter);
				new_topic_filter.back()->second.value.emplace(std::forward<K>(key), std::forward<V>(value));
				this->increase_map_size();
				return std::make_pair(this->path_to_handle(std::move(new_topic_filter)), true);
			}
			else
			{
				auto& subscription_set = path.back()->second.value;

				auto insert_result = subscription_set.insert_or_assign(std::forward<K>(key), std::forward<V>(value));
				if (insert_result.second)
				{
					this->increase_subscriptions(path);
					this->increase_map_size();
				}
				return std::make_pair(this->path_to_handle(std::move(path)), insert_result.second);
			}
		}

		// Insert a key => value with a handle to the topic filter
		// returns the handle and true if key was inserted, false if key was updated
		template <typename K, typename V>
		inline std::pair<handle, bool> insert_or_assign(handle const &h, K&& key, V&& value)
		{
			auto h_iter = this->get_key(h);
			if (h_iter == this->end())
			{
				this->throw_invalid_handle();
			}

			auto& subscription_set = h_iter->second.value;

			auto insert_result = subscription_set.insert_or_assign(std::forward<K>(key), std::forward<V>(value));
			if (insert_result.second)
			{
				this->increase_subscriptions(h);
				this->increase_map_size();
			}
			return std::make_pair(h, insert_result.second);
		}

		// Remove a value at the specified handle
		// returns the number of removed elements
		inline std::size_t erase(handle const &h, Key const& key)
		{
			// Find the handle in the map
			auto h_iter = this->get_key(h);
			if (h_iter == this->end())
			{
				this->throw_invalid_handle();
			}

			// Remove the specified value
			auto result = h_iter->second.value.erase(key);
			if (result)
			{
				std::vector<map_type_iterator> v = this->handle_to_iterators(h);
				this->remove_topic_filter(v);
				this->decrease_map_size();
			}

			return result;
		}

		// Remove a value at the specified topic filter
		// returns the number of removed elements
		inline std::size_t erase(std::string_view topic_filter, Key const& key)
		{
			// Find the topic filter in the map
			auto path = this->find_topic_filter(topic_filter);
			if (path.empty())
			{
				return 0;
			}

			// Remove the specified value
			auto result = path.back()->second.value.erase(key);
			if (result)
			{
				this->decrease_map_size();
				this->remove_topic_filter(path);
			}

			return result;
		}

		// Find all topic filters that match the specified topic
		template<typename Output>
		inline void find(std::string_view topic, Output&& callback) const
		{
			this->find_match(topic, [&callback](Container const &values) mutable
			{
				for (auto const& i : values)
				{
					callback(i.first, i.second);
				}
			});
		}

		// Find all topic filters that match and allow modification
		template<typename Output>
		inline void modify(std::string_view topic, Output&& callback)
		{
			this->modify_match(topic, [&callback](Container &values) mutable
			{
				for (auto& i : values)
				{
					callback(i.first, i.second);
				}
			});
		}

		template<typename Output>
		inline void dump(Output &out)
		{
			out << "Root node id: " << this->root_node_id << std::endl;
			for (auto const& i : this->get_map())
			{
				out << "(" << i.first.first << ", " << i.first.second << "): id: "
					<< i.second.id << ", size: " << i.second.value.size()
					<< ", value: " << i.second.count.value << std::endl;
			}
		}
	};

	template<class session_t>
	struct subscription_entry
	{
		explicit subscription_entry(
			session_t* session,
			mqtt::subscription sub,
			mqtt::v5::properties_set props = mqtt::v5::properties_set{}
		)
			: session(session       )
			, sub    (std::move(sub))
			, props  (std::move(props))
		{
		}

		inline std::string_view share_name  () { return sub.share_name  (); }
		inline std::string_view topic_filter() { return sub.topic_filter(); }

		// 
		session_t                * session = nullptr;

		// subscription info
		mqtt::subscription         sub;

		// subscription properties
		mqtt::v5::properties_set   props;
	};
}

#endif // !__ASIO2_MQTT_SUBSCRIPTION_MAP_HPP__
