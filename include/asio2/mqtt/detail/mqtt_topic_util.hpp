/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * refrenced from : mqtt_cpp/include/mqtt/topic_filter_tokenizer.hpp
 */

#ifndef __ASIO2_MQTT_TOPIC_UTIL_HPP__
#define __ASIO2_MQTT_TOPIC_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>

#include <asio2/base/detail/util.hpp>

// custom specialization of std::hash can be injected in namespace std
namespace std
{
	template<> struct hash<std::pair<std::size_t, std::string_view>>
	{
		typedef std::pair<std::size_t, std::string_view> argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& pair) const noexcept
		{
			std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
				(const unsigned char *)(&(pair.first)), sizeof(std::size_t));
			return asio2::detail::fnv1a_hash<std::size_t>(v,
				(const unsigned char *)(pair.second.data()), pair.second.size());
		}
	};
	template<> struct hash<std::pair<std::size_t, std::string>>
	{
		typedef std::pair<std::size_t, std::string> argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& pair) const noexcept
		{
			std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
				(const unsigned char *)(&(pair.first)), sizeof(std::size_t));
			return asio2::detail::fnv1a_hash<std::size_t>(v,
				(const unsigned char *)(pair.second.data()), pair.second.size());
		}
	};
	template<> struct hash<std::pair<std::string_view, std::string_view>>
	{
		typedef std::pair<std::string_view, std::string_view> argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& pair) const noexcept
		{
			std::size_t v = asio2::detail::fnv1a_hash<std::size_t>(
				(const unsigned char *)(pair.first.data()), pair.first.size());
			return asio2::detail::fnv1a_hash<std::size_t>(v,
				(const unsigned char *)(pair.second.data()), pair.second.size());
		}
	};
}

namespace asio2::mqtt
{
	template<typename = void>
	inline bool is_topic_name_valid(std::string_view str)
	{
		// All Topic Names and Topic Filters MUST be at least one character long [MQTT-4.7.3-1]
		return ((!str.empty()) && (str.find_first_of("+#") == std::string_view::npos));
	}

	template<typename = void>
	inline std::pair<std::string_view, std::string_view> parse_topic_filter(std::string_view topic_filter)
	{
		using namespace std::literals;

		constexpr std::string_view shared_prefix = "$share/"sv;

		if (topic_filter.empty())
			return { ""sv, ""sv };

		if (topic_filter.substr(0, shared_prefix.size()) != shared_prefix)
			return { ""sv, std::move(topic_filter) };

		// Remove $share/
		topic_filter.remove_prefix(shared_prefix.size());

		// This is the '/' seperating the subscription group from the actual topic_filter.
		std::string_view::size_type idx = topic_filter.find('/');
		if (idx == std::string_view::npos)
			return { ""sv, ""sv };

		// We return the share_name and the topic_filter as buffers that point to the same
		// storage. So we grab the substr for "share", and then remove it from topic_filter.
		std::string_view share_name = topic_filter.substr(0, idx);
		topic_filter.remove_prefix(idx + 1);

		if (share_name.empty() || topic_filter.empty())
			return { ""sv, ""sv };

		return { std::move(share_name), std::move(topic_filter) };
	}

	static constexpr char topic_filter_separator = '/';

	template<typename Iterator, typename Output>
	inline std::size_t topic_filter_tokenizer(Iterator&& first, Iterator&& last, Output&& write)
	{
		if (first >= last)
		{
			ASIO2_ASSERT(false);
			return static_cast<std::size_t>(0);
		}
		std::size_t count = static_cast<std::size_t>(1);
		auto iter = std::find(first, last, topic_filter_separator);
		while (write(asio2::detail::to_string_view(first, iter)) && iter != last)
		{
			first = std::next(iter);
			if (first >= last)
				break;
			iter = std::find(first, last, topic_filter_separator);
			++count;
		}
		return count;
	}

	template<typename Output>
	inline std::size_t topic_filter_tokenizer(std::string_view str, Output&& write)
	{
		return topic_filter_tokenizer(std::begin(str), std::end(str), std::forward<Output>(write));
	}
}

#endif // !__ASIO2_MQTT_TOPIC_UTIL_HPP__
