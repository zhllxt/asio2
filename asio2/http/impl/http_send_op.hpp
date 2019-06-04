/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_SEND_OP_HPP__
#define __ASIO2_HTTP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#include <asio2/http/detail/http_util.hpp>

namespace asio2::detail
{
	template<class derived_t, class body_t, class buffer_t, bool isSession>
	class http_send_op
	{
	public:
		using body_type = body_t;
		using buffer_type = buffer_t;

		/**
		 * @constructor
		 */
		http_send_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~http_send_op() = default;

	protected:
		template<bool isRequest, class Stream, class ConstBufferSequence>
		inline bool _http_send(Stream& stream, ConstBufferSequence buffer)
		{
			http::message<isRequest, body_type> msg{};
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}
			error_code ec;
			if constexpr (isRequest)
				msg = http::make_request<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			else
				msg = http::make_response<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			if (ec)
			{
				set_last_error(ec);
				return false;
			}
			http::serializer<isRequest, body_type> sr{ msg };
			http::write(stream, sr, ec);
			set_last_error(ec);
			return (!ec.operator bool());
		}

		template<bool isRequest, class Stream, class ConstBufferSequence, class Callback>
		inline bool _http_send(Stream& stream, ConstBufferSequence buffer, Callback& fn)
		{
			http::message<isRequest, body_type> msg{};
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}
			error_code ec;
			if constexpr (isRequest)
				msg = http::make_request<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			else
				msg = http::make_response<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			if (ec)
			{
				set_last_error(ec);
				callback_helper::call(fn, 0);
				return false;
			}
			http::serializer<isRequest, body_type> sr{ msg };
			std::size_t sent_bytes = http::write(stream, sr, ec);
			set_last_error(ec);
			callback_helper::call(fn, sent_bytes);
			return (!ec.operator bool());
		}

		template<bool isRequest, class Stream, class ConstBufferSequence>
		inline bool _http_send(Stream& stream, ConstBufferSequence buffer,
			std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			http::message<isRequest, body_type> msg{};
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}
			error_code ec;
			if constexpr (isRequest)
				msg = http::make_request<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			else
				msg = http::make_response<body_type>(std::string_view(
					reinterpret_cast<std::string_view::const_pointer>(buffer.data()), buffer.size()), ec);
			if (ec)
			{
				set_last_error(ec);
				promise.set_value(std::pair<error_code, std::size_t>(ec, 0));
				return false;
			}
			http::serializer<isRequest, body_type> sr{ msg };
			std::size_t sent_bytes = http::write(stream, sr, ec);
			set_last_error(ec);
			promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));
			return (!ec.operator bool());
		}

		template<bool isRequest, class Body, class Fields, class Stream>
		inline bool _http_send(Stream& stream, http::message<isRequest, Body, Fields>& msg)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}
			error_code ec;
			// Write the response
			http::serializer<isRequest, Body, Fields> sr{ msg };
			http::write(stream, sr, ec);
			set_last_error(ec);
			return (!ec.operator bool());
		}

		template<bool isRequest, class Body, class Fields, class Stream, class Callback>
		inline bool _http_send(Stream& stream, http::message<isRequest, Body, Fields>& msg, Callback& fn)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}
			error_code ec;
			// Write the response
			http::serializer<isRequest, Body, Fields> sr{ msg };
			std::size_t sent_bytes = http::write(stream, sr, ec);
			set_last_error(ec);
			callback_helper::call(fn, sent_bytes);
			return (!ec.operator bool());
		}

		template<bool isRequest, class Body, class Fields, class Stream>
		inline bool _http_send(Stream& stream, http::message<isRequest, Body, Fields>& msg,
			std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}
			error_code ec;
			// Write the response
			http::serializer<isRequest, Body, Fields> sr{ msg };
			std::size_t sent_bytes = http::write(stream, sr, ec);
			set_last_error(ec);
			promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));
			return (!ec.operator bool());
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_HTTP_SEND_OP_HPP__

#endif
