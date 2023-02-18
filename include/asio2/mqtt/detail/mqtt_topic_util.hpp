/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

namespace asio2::mqtt
{
	template<typename = void>
	inline bool is_topic_name_valid(std::string_view str)
	{
		// All Topic Names and Topic Filters MUST be at least one character long [MQTT-4.7.3-1]
		return ((!str.empty()) && (str.find_first_of("+#") == std::string_view::npos));
	}

	/**
	 * @return std::pair<shared_name, topic_filter>
	 */
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

	template<typename Iterator>
	inline Iterator topic_filter_tokenizer_next(Iterator first, Iterator last)
	{
		return std::find(first, last, topic_filter_separator);
	}

	template<typename Iterator, typename Function>
	inline std::size_t topic_filter_tokenizer(Iterator&& first, Iterator&& last, Function&& callback)
	{
		if (first >= last)
		{
			ASIO2_ASSERT(false);
			return static_cast<std::size_t>(0);
		}
		std::size_t count = static_cast<std::size_t>(1);
		auto iter = topic_filter_tokenizer_next(first, last);
		while (callback(asio2::detail::to_string_view(first, iter)) && iter != last)
		{
			first = std::next(iter);
			if (first >= last)
				break;
			iter = topic_filter_tokenizer_next(first, last);
			++count;
		}
		return count;
	}

	template<typename Function>
	inline std::size_t topic_filter_tokenizer(std::string_view str, Function&& callback)
	{
		return topic_filter_tokenizer(std::begin(str), std::end(str), std::forward<Function>(callback));
	}

	// TODO: Technically this function is simply wrong, since it's treating the
	// topic pattern as if it were an ASCII sequence.
	// To make this function correct per the standard, it would be necessary
	// to conduct the search for the wildcard characters using a proper
	// UTF-8 API to avoid problems of interpreting parts of multi-byte characters
	// as if they were individual ASCII characters
	constexpr inline bool validate_topic_filter(std::string_view topic_filter)
	{
		// Confirm the topic pattern is valid before registering it.
		// Use rules from http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718106

		// All Topic Names and Topic Filters MUST be at least one character long
		// Topic Names and Topic Filters are UTF-8 Encoded Strings; they MUST NOT encode to more than 65,535 bytes
		if (topic_filter.empty() || (topic_filter.size() > (std::numeric_limits<std::uint16_t>::max)()))
		{
			return false;
		}

		for (std::string_view::size_type idx = topic_filter.find_first_of(std::string_view("\0+#", 3));
			std::string_view::npos != idx;
			idx = topic_filter.find_first_of(std::string_view("\0+#", 3), idx + 1))
		{
			ASIO2_ASSERT(
				('\0' == topic_filter[idx]) ||
				('+'  == topic_filter[idx]) ||
				('#'  == topic_filter[idx])
			);
			if ('\0' == topic_filter[idx])
			{
				// Topic Names and Topic Filters MUST NOT include the null character (Unicode U+0000)
				return false;
			}
			else if ('+' == topic_filter[idx])
			{
				/*
				 * Either must be the first character,
				 * or be preceeded by a topic seperator.
				 */
				if ((0 != idx) && ('/' != topic_filter[idx - 1]))
				{
					return false;
				}

				/*
				 * Either must be the last character,
				 * or be followed by a topic seperator.
				 */
				if ((topic_filter.size() - 1 != idx) && ('/' != topic_filter[idx + 1]))
				{
					return false;
				}
			}
			// multilevel wildcard
			else if ('#' == topic_filter[idx])
			{
				/*
				 * Must be absolute last character.
				 * Must only be one multi level wild card.
				 */
				if (idx != topic_filter.size() - 1)
				{
					return false;
				}

				/*
				 * If not the first character, then the
				 * immediately preceeding character must
				 * be a topic level separator.
				 */
				if ((0 != idx) && ('/' != topic_filter[idx - 1]))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}
}

#endif // !__ASIO2_MQTT_TOPIC_UTIL_HPP__
