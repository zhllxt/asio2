/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/session_state.hpp
 */

#ifndef __ASIO2_MQTT_SESSION_STATE_HPP__
#define __ASIO2_MQTT_SESSION_STATE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <optional>
#include <chrono>

#include <asio2/base/detail/util.hpp>

#include <asio2/mqtt/detail/mqtt_inflight_message.hpp>
#include <asio2/mqtt/detail/mqtt_offline_message.hpp>
#include <asio2/mqtt/detail/mqtt_shared_target.hpp>

namespace asio2::mqtt
{
	class session_state
	{
	public:
		session_state()
		{

		}
		~session_state()
		{

		}

		void clean()
		{
			//inflight_messages_.clear();
			offline_messages_.clear();
			qos2_publish_processed_.clear();
			//shared_targets_.erase(*this);
			//unsubscribe_all();
		}

		void exactly_once_start(mqtt::two_byte_integer::value_type packet_id)
		{
			qos2_publish_processed_.insert(packet_id);
		}

		bool exactly_once_processing(mqtt::two_byte_integer::value_type packet_id) const
		{
			return qos2_publish_processed_.find(packet_id) != qos2_publish_processed_.end();
		}

		void exactly_once_finish(mqtt::two_byte_integer::value_type packet_id)
		{
			qos2_publish_processed_.erase(packet_id);
		}

	protected:
		//std::shared_ptr<asio::steady_timer> will_expiry_timer_;
		//std::optional<MQTT_NS::will> will_value_;

		//sub_con_map& subs_map_;
		//shared_target& shared_targets_;
		//con_sp_t con_;
		std::string_view client_id_;

		std::optional<std::chrono::steady_clock::duration> will_delay_;
		std::optional<std::chrono::steady_clock::duration> session_expiry_interval_;
		std::shared_ptr<asio::steady_timer>                session_expiry_timer_;

		//inflight_messages inflight_messages_;
		std::set<mqtt::two_byte_integer::value_type> qos2_publish_processed_;

		offline_messages<omnode> offline_messages_;

		//std::set<sub_con_map::handle> handles_; // to efficient remove
	};
}

#endif // !__ASIO2_MQTT_SESSION_STATE_HPP__
