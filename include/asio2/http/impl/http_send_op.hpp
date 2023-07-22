/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_SEND_OP_HPP__
#define __ASIO2_HTTP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#include <asio2/http/detail/http_util.hpp>
#include <asio2/http/request.hpp>
#include <asio2/http/response.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class http_send_op
	{
	public:
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @brief constructor
		 */
		http_send_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~http_send_op() = default;
	
	protected:
		template<bool isRequest, class Body, class Fields>
		inline void _check_http_message(http::message<isRequest, Body, Fields>& msg)
		{
			// https://datatracker.ietf.org/doc/html/rfc2616#section-14.13
			// If an http message header don't has neither "Content-Length" nor "Transfer-Encoding"(chunk)
			// Then the receiver may not be able to parse the http message normally.
			if (!msg.chunked())
			{
				if (msg.find(http::field::content_length) == msg.end())
				{
					http::try_prepare_payload(msg);
				}
			}
		}

	protected:
		template<class Data, class Callback>
		inline bool _http_send(Data& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return derive._tcp_send(data, std::forward<Callback>(callback));
		}

		template<bool isRequest, class Body, class Fields, class Callback>
		inline bool _http_send(http::message<isRequest, Body, Fields>& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive._check_http_message(data);

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			http::async_write(derive.stream(), data, make_allocator(derive.wallocator(),
			[&derive, callback = std::forward<Callback>(callback)]
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

		template<class Body, class Fields, class Callback>
		inline bool _http_send(detail::http_request_impl_t<Body, Fields>& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive._check_http_message(data.base());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			http::async_write(derive.stream(), data.base(), make_allocator(derive.wallocator(),
			[&derive, callback = std::forward<Callback>(callback)]
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

		template<class Body, class Fields, class Callback>
		inline bool _http_send(detail::http_response_impl_t<Body, Fields>& data, Callback&& callback)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			derive._check_http_message(data.base());

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			http::async_write(derive.stream(), data.base(), make_allocator(derive.wallocator(),
			[&derive, callback = std::forward<Callback>(callback)]
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

	protected:
	};
}

#endif // !__ASIO2_HTTP_SEND_OP_HPP__
