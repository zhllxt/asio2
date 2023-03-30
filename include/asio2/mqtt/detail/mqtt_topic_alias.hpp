/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_TOPIC_ALIAS_HPP__
#define __ASIO2_MQTT_TOPIC_ALIAS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <unordered_map>

#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class derived_t, class args_t>
	class mqtt_topic_alias_t
	{
		friend derived_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	protected:
		using self = mqtt_topic_alias_t<derived_t, args_t>;

		/**
		 * @brief constructor
		 */
		mqtt_topic_alias_t()
		{
		}

		/**
		 * @brief destructor
		 */
		~mqtt_topic_alias_t() = default;

	public:
		template<class String>
		inline derived_t& push_topic_alias(std::uint16_t alias_value, String&& topic_name)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio2::unique_locker guard(this->topic_alias_mutex_);

			topic_aliases_[alias_value] = detail::to_string(std::forward<String>(topic_name));

			return static_cast<derived_t&>(*this);
		}

		inline bool find_topic_alias(std::uint16_t alias_value, std::string& topic_name)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio2::shared_locker guard(this->topic_alias_mutex_);

			auto iter = topic_aliases_.find(alias_value);

			if (iter != topic_aliases_.end())
			{
				topic_name = iter->second;
				return true;
			}

			return false;
		}

	protected:
		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer  topic_alias_mutex_;

		/// 
		std::unordered_map<std::uint16_t, std::string> topic_aliases_ ASIO2_GUARDED_BY(topic_alias_mutex_);
	};
}

#endif // !__ASIO2_MQTT_TOPIC_ALIAS_HPP__
