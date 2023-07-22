/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MQTT_SEND_OP_HPP__
#define __ASIO2_MQTT_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>
#include <vector>
#include <variant>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

#include <asio2/mqtt/message.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class mqtt_send_op
	{
	public:
		/**
		 * @brief constructor
		 */
		mqtt_send_op() {}

		/**
		 * @brief destructor
		 */
		~mqtt_send_op() = default;

	protected:
		template<class Data, class Callback>
		inline bool _mqtt_send(Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// why don't use std::string ?
			// beacuse std::string has a SSO(Small String Optimization) mechanism
			// https://stackoverflow.com/questions/34788789/disable-stdstrings-sso
			// std::string str;
			// str.reserve(sizeof(str) + 1);

			// Test whether the vector has SSO mechanism
		#if defined(_DEBUG) || defined(DEBUG)
			static bool flag = false;
			if (flag == false)
			{
				flag = true;

				std::vector<char> v1{ 'a','b','c' };

				std::string_view  sv{ v1.data(),v1.size() };

				std::vector<char> v2 = std::move(v1);

				ASIO2_ASSERT(sv.data() == v2.data());
			}
		#endif

			std::vector<char> binary;

			using data_type = typename detail::remove_cvref_t<Data>;

			if constexpr (std::is_same_v<mqtt::message, data_type>)
			{
				std::visit([&binary](auto& message) mutable
				{
					binary.reserve(message.required_size());
					message.serialize(binary);
				}, data);
			}
			else if constexpr (detail::is_template_instance_of_v<std::variant, data_type>)
			{
				std::visit([&binary](auto& message) mutable
				{
					binary.reserve(message.required_size());
					message.serialize(binary);
				}, data);
			}
			else
			{
				binary.reserve(data.required_size());
				data.serialize(binary);
			}

			auto buffer = asio::buffer(binary);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			asio::async_write(derive.stream(), buffer,
				make_allocator(derive.wallocator(), [&derive,
					binary = std::move(binary), callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				set_last_error(ec);

				callback(ec, bytes_sent);

				if (ec)
				{
					// must stop, otherwise re-sending will cause body confusion
					if (derive.state_ == state_t::started)
					{
						derive._do_disconnect(ec, derive.selfptr());
					}
				}
			}));

			return true;
		}
	};
}

#endif // !__ASIO2_MQTT_SEND_OP_HPP__
