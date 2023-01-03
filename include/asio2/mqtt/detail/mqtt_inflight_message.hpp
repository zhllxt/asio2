/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/inflight_message.hpp
 */

#ifndef __ASIO2_MQTT_INFLIGHT_MESSAGE_HPP__
#define __ASIO2_MQTT_INFLIGHT_MESSAGE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <variant>

#include <asio2/base/iopool.hpp>

#include <asio2/mqtt/message.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>

namespace asio2::mqtt
{
	//class inflight_messages
	//{
	//public:
	//	void insert(
	//		store_message_variant msg,
	//		any life_keeper,
	//		std::shared_ptr<asio::steady_timer> tim_message_expiry
	//	)
	//	{
	//		messages_.emplace_back(
	//			std::move(msg),
	//			std::move(life_keeper),
	//			std::move(tim_message_expiry)
	//		);
	//	}

	//	void send_all_messages(endpoint_t& ep)
	//	{
	//		for (auto const& ifm : messages_)
	//		{
	//			ifm.send(ep);
	//		}
	//	}

	//	void clear()
	//	{
	//		messages_.clear();
	//	}

	//	template <typename Tag>
	//	decltype(auto) get()
	//	{
	//		return messages_.get<Tag>();
	//	}

	//	template <typename Tag>
	//	decltype(auto) get() const
	//	{
	//		return messages_.get<Tag>();
	//	}

	//protected:
	//	using mi_inflight_message = mi::multi_index_container<
	//		imnode,
	//		mi::indexed_by<
	//		mi::sequenced<
	//		mi::tag<tag_seq>
	//		>,
	//		mi::ordered_unique<
	//		mi::tag<tag_pid>,
	//		BOOST_MULTI_INDEX_CONST_MEM_FUN(imnode, packet_id_t, packet_id)
	//		>,
	//		mi::ordered_non_unique<
	//		mi::tag<tag_tim>,
	//		BOOST_MULTI_INDEX_MEMBER(imnode, std::shared_ptr<asio::steady_timer>, message_expiry_timer_)
	//		>
	//		>
	//	>;

	//	mi_inflight_message messages_;
	//};

	//struct imnode
	//{
	//	template<class Message>
	//	imnode(Message&& msg,
	//		any life_keeper,
	//		std::shared_ptr<asio::steady_timer> tim_message_expiry
	//	)
	//		: message(std::forward<Message>(msg))
	//		, life_keeper_{ std::move(life_keeper) }
	//		, message_expiry_timer_{ std::move(tim_message_expiry) }
	//	{}

	//	packet_id_t packet_id() const
	//	{
	//		return std::visit(make_lambda_visitor(
	//			[](auto const& m) {
	//			return m.packet_id();
	//		}
	//		),
	//			msg_
	//			);
	//	}

	//	void send(endpoint_t& ep) const
	//	{
	//		optional<store_message_variant> msg_opt;
	//		if (message_expiry_timer_) {
	//			MQTT_NS::visit(
	//				make_lambda_visitor(
	//					[&](v5::basic_publish_message<sizeof(packet_id_t)> const& m) {
	//				auto updated_msg = m;
	//				auto d =
	//					std::chrono::duration_cast<std::chrono::seconds>(
	//						message_expiry_timer_->expiry() - std::chrono::steady_clock::now()
	//						).count();
	//				if (d < 0) d = 0;
	//				updated_msg.update_prop(
	//					v5::property::message_expiry_interval(
	//						static_cast<uint32_t>(d)
	//					)
	//				);
	//				msg_opt.emplace(std::move(updated_msg));
	//			},
	//					[](auto const&) {
	//			}
	//				),
	//				msg_
	//				);
	//		}
	//		// packet_id_exhausted never happen because inflight message has already
	//		// allocated packet_id at the previous connection.
	//		// In  send_store_message(), packet_id is registered.
	//		ep.send_store_message(msg_opt ? msg_opt.value() : msg_, life_keeper_);
	//	}

	//protected:
	//	mqtt::message message;

	//	//any life_keeper_;
	//	std::shared_ptr<asio::steady_timer> message_expiry_timer_;
	//};
}

#endif // !__ASIO2_MQTT_INFLIGHT_MESSAGE_HPP__
