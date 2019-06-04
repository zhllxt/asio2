/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_SEND_OP_HPP__
#define __ASIO2_UDP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class udp_send_op
	{
	public:
		/**
		 * @constructor
		 */
		udp_send_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~udp_send_op() = default;

	protected:
		template<class ConstBufferSequence>
		inline bool _udp_send(ConstBufferSequence buffer)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}
			error_code ec;
			derive.stream().send(buffer, 0, ec);
			set_last_error(ec);
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _udp_send(ConstBufferSequence buffer, Callback& fn)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}
			error_code ec;
			std::size_t sent_bytes = derive.stream().send(buffer, 0, ec);
			set_last_error(ec);
			callback_helper::call(fn, sent_bytes);
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence>
		inline bool _udp_send(ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}
			error_code ec;
			std::size_t sent_bytes = derive.stream().send(buffer, 0, ec);
			set_last_error(ec);
			promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence>
		inline bool _udp_send_to(const asio::ip::udp::endpoint& endpoint, ConstBufferSequence buffer)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}
			error_code ec;
			derive.stream().send_to(buffer, endpoint, 0, ec);
			set_last_error(ec);
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _udp_send_to(const asio::ip::udp::endpoint& endpoint, ConstBufferSequence buffer, Callback& fn)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}
			error_code ec;
			std::size_t sent_bytes = derive.stream().send_to(buffer, endpoint, 0, ec);
			set_last_error(ec);
			callback_helper::call(fn, sent_bytes);
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence>
		inline bool _udp_send_to(const asio::ip::udp::endpoint& endpoint, ConstBufferSequence buffer,
			std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}
			error_code ec;
			std::size_t sent_bytes = derive.stream().send_to(buffer, endpoint, 0, ec);
			set_last_error(ec);
			promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));
			return (!ec.operator bool());
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_UDP_SEND_OP_HPP__
