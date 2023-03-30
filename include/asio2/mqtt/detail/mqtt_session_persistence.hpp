/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_SESSION_PERSISTENCE_HPP__
#define __ASIO2_MQTT_SESSION_PERSISTENCE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <unordered_map>

#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/shared_mutex.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class session_t, class args_t>
	class mqtt_session_persistence
	{
		friend session_t;

		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		using self = mqtt_session_persistence<session_t, args_t>;

		/**
		 * @brief constructor
		 */
		mqtt_session_persistence()
		{
		}

		/**
		 * @brief destructor
		 */
		~mqtt_session_persistence() = default;

	public:
		/**
		 * @brief add a mqtt session
		 */
		template<class StringT>
		inline self& push_mqtt_session(StringT&& clientid, std::shared_ptr<session_t> session_ptr)
		{
			asio2::unique_locker guard(this->session_persistence_mutex_);

			this->mqtt_sessions_map_[detail::to_string(std::forward<StringT>(clientid))] = std::move(session_ptr);

			return (*this);
		}

		/**
		 * @brief find the mqtt session by client id
		 */
		inline std::shared_ptr<session_t> find_mqtt_session(const std::string& clientid)
		{
			asio2::shared_locker guard(this->session_persistence_mutex_);

			auto iter = this->mqtt_sessions_map_.find(clientid);

			return iter == this->mqtt_sessions_map_.end() ? nullptr : iter->second;
		}

		/**
		 * @brief find the mqtt session by client id
		 */
		inline std::shared_ptr<session_t> find_mqtt_session(const std::string_view& clientid)
		{
			return this->find_mqtt_session(std::string(clientid));
		}

		/**
		 * @brief remove the mqtt session by client id
		 */
		inline bool erase_mqtt_session(const std::string& clientid)
		{
			asio2::unique_locker guard(this->session_persistence_mutex_);

			return this->mqtt_sessions_map_.erase(clientid) > 0;
		}

		/**
		 * @brief remove the mqtt session by client id
		 */
		inline bool erase_mqtt_session(const std::string_view& clientid)
		{
			return this->erase_mqtt_session(std::string(clientid));
		}

		/**
		 * @brief remove the mqtt session by client id and session itself
		 */
		inline bool erase_mqtt_session(const std::string& clientid, session_t* p)
		{
			asio2::unique_locker guard(this->session_persistence_mutex_);

			auto iter = this->mqtt_sessions_map_.find(clientid);

			if (iter != this->mqtt_sessions_map_.end())
			{
				if (iter->second.get() == p)
				{
					this->mqtt_sessions_map_.erase(iter);
					return true;
				}
			}

			return false;
		}

		/**
		 * @brief remove the mqtt session by client id and session itself
		 */
		inline bool erase_mqtt_session(const std::string_view& clientid, session_t* p)
		{
			return this->erase_mqtt_session(std::string(clientid), p);
		}

		/**
		 * @brief remove all mqtt sessions
		 */
		inline self& clear_mqtt_sessions()
		{
			asio2::unique_locker guard(this->session_persistence_mutex_);

			this->mqtt_sessions_map_.clear();

			return (*this);
		}

	protected:
		/// use rwlock to make thread safe
		mutable asio2::shared_mutexer  session_persistence_mutex_;

		/// 
		std::unordered_map<std::string, std::shared_ptr<session_t>> mqtt_sessions_map_ ASIO2_GUARDED_BY(session_persistence_mutex_);
	};
}

#endif // !__ASIO2_MQTT_SESSION_PERSISTENCE_HPP__
