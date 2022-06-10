/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_MQTT_TOPIC_ALIAS_HPP__
#define __ASIO2_MQTT_TOPIC_ALIAS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <unordered_map>

#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/mqtt_protocol_util.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class caller_t>
	class mqtt_topic_alias_t
	{
		friend caller_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using self = mqtt_topic_alias_t<caller_t>;

		/**
		 * @constructor
		 */
		mqtt_topic_alias_t()
		{
		}

		/**
		 * @destructor
		 */
		~mqtt_topic_alias_t() = default;

		template<class String>
		inline self& push_topic_alias(std::uint16_t alias_value, String&& topic_name)
		{
			asio2_unique_lock guard(this->topic_aliases_mtx_);

			topic_aliases_[alias_value] = detail::to_string(std::forward<String>(topic_name));

			return (*this);
		}

		inline bool find_topic_alias(std::uint16_t alias_value, std::string_view& topic_name)
		{
			asio2_shared_lock guard(this->topic_aliases_mtx_);

			auto iter = topic_aliases_.find(alias_value);

			if (iter != topic_aliases_.end())
			{
				topic_name = iter->second;
				return true;
			}

			return false;
		}

	protected:
		mutable asio2_shared_mutex                     topic_aliases_mtx_;
		std::unordered_map<std::uint16_t, std::string> topic_aliases_;
	};
}

#endif // !__ASIO2_MQTT_TOPIC_ALIAS_HPP__
