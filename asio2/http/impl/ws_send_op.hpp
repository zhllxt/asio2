/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_WS_SEND_OP_HPP__
#define __ASIO2_WS_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class ws_send_op
	{
	public:
		/**
		 * @constructor
		 */
		ws_send_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~ws_send_op() = default;

	protected:
		template<class ConstBufferSequence>
		inline bool _ws_send(ConstBufferSequence buffer)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}

			error_code ec;
			derive.ws_stream().write(buffer, ec);
			set_last_error(ec);
			return (!ec.operator bool());
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _ws_send(ConstBufferSequence buffer, Callback& fn)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}

			error_code ec;
			std::size_t sent_bytes = derive.ws_stream().write(buffer, ec);
			set_last_error(ec);
			callback_helper::call(fn, sent_bytes);

			return (!ec.operator bool());
		}

		template<class ConstBufferSequence>
		inline bool _ws_send(ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}

			error_code ec;
			std::size_t sent_bytes = derive.ws_stream().write(buffer, ec);
			set_last_error(ec);
			promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));

			return (!ec.operator bool());
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_WS_SEND_OP_HPP__

#endif
