/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/offline_message.hpp
 */

#ifndef __ASIO2_MQTT_OFFLINE_MESSAGE_HPP__
#define __ASIO2_MQTT_OFFLINE_MESSAGE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <list>
#include <algorithm>
#include <variant>

#include <asio2/base/iopool.hpp>

#include <asio2/mqtt/message.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	// The offline message structure holds messages that have been published on a
	// topic that a not-currently-connected client is subscribed to.
	// When a new connection is made with the client id for this saved data,
	// these messages will be published to that client, and only that client.

	template<class Value>
	class offline_messages
	{
	public:
		 offline_messages() = default;
		~offline_messages() = default;

		//void send_all(endpoint_t& ep)
		//{
		//	auto& idx = messages_.get<tag_seq>();
		//	while (!idx.empty())
		//	{
		//		if (idx.front().send(ep))
		//		{
		//			idx.pop_front();
		//		}
		//		else
		//		{
		//			break;
		//		}
		//	}
		//}

		//void send_by_packet_id_release(endpoint_t& ep)
		//{
		//	auto& idx = messages_.get<tag_seq>();
		//	while (!idx.empty())
		//	{
		//		if (idx.front().send(ep))
		//		{
		//			// if packet_id is consumed, then finish
		//			idx.pop_front();
		//		}
		//		else
		//		{
		//			break;
		//		}
		//	}
		//}

		//bool send(endpoint_t& ep) const
		//{
		//	auto props = props_;
		//	if (message_expiry_timer_)
		//	{
		//		auto d = std::chrono::duration_cast<std::chrono::seconds>(
		//			message_expiry_timer_->expiry() - std::chrono::steady_clock::now()).count();

		//		if (d < 0)
		//			d = 0;

		//		set_property<v5::property::message_expiry_interval>(
		//			props,
		//			v5::property::message_expiry_interval(
		//				static_cast<uint32_t>(d)));
		//	}

		//	mqtt::qos_type qos_value = publish_.qos();
		//	if (qos_value == mqtt::qos_type::at_least_once || qos_value == mqtt::qos_type::exactly_once)
		//	{
		//		if (auto pid = ep.acquire_unique_packet_id_no_except())
		//		{
		//			ep.publish(pid.value(), topic_, contents_, pubopts_, std::move(props));
		//			return true;
		//		}
		//	}
		//	else
		//	{
		//		ep.publish(topic_, contents_, pubopts_, std::move(props));
		//		return true;
		//	}

		//	return false;
		//}

		void clear()
		{
			messages_.clear();
		}

		bool empty() const
		{
			return messages_.empty();
		}

		template<class Message>
		void push_back(asio::io_context& ioc, Message&& msg)
		{
			using message_type = typename asio2::detail::remove_cvref_t<Message>;

			auto it = messages_.emplace(messages_.end(), std::forward<Message>(msg), nullptr);

			if constexpr (std::is_same_v<message_type, mqtt::v5::publish>)
			{
				mqtt::v5::message_expiry_interval* mei =
					msg.properties().template get_if<mqtt::v5::message_expiry_interval>();
				if (mei)
				{
					std::shared_ptr<asio::steady_timer> expiry_timer =
						std::make_shared<asio::steady_timer>(ioc, std::chrono::seconds(mei->value()));
					expiry_timer->async_wait(
					[this, it, wp = std::weak_ptr<asio::steady_timer>(expiry_timer)](error_code ec) mutable
					{
						if (auto sp = wp.lock())
						{
							if (!ec)
							{
								messages_.erase(it);
							}
						}
					});

					it->message_expiry_timer = std::move(expiry_timer);
				}
			}
			else
			{
				asio2::ignore_unused(ioc, msg, it);
			}
		}

		template<class Callback>
		void for_each(Callback&& cb)
		{
			for (auto& v : messages_)
			{
				cb(v);
			}
		}

	protected:
		/// 
		std::list<Value> messages_;
	};

	struct omnode
	{
	public:
		template<class Message>
		explicit omnode(Message&& msg, std::shared_ptr<asio::steady_timer> expiry_timer)
			: message(std::forward<Message>(msg))
			, message_expiry_timer(std::move(expiry_timer))
		{
		}

		mqtt::message message;
		std::shared_ptr<asio::steady_timer> message_expiry_timer;
	};
}

#endif // !__ASIO2_MQTT_OFFLINE_MESSAGE_HPP__
