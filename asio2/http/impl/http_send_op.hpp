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
		template<bool isRequest, class Stream, class Data, class Callback>
		inline bool _http_send(Stream& stream, Data& data, Callback&& callback)
		{
			using msg_type = std::remove_cv_t<std::remove_reference_t<decltype(data())>>;
#if defined(ASIO2_SEND_CORE_ASYNC)
			http::async_write(stream, const_cast<msg_type&>(data()), asio::bind_executor(derive.io().strand(),
				make_allocator(derive.wallocator(),
					[this, p = derive.selfptr(), callback = std::forward<Callback>(callback)]
			(const error_code& ec, std::size_t bytes_sent) mutable
			{
				set_last_error(ec);

				callback(ec, bytes_sent);

				derive._send_dequeue();
			})));
			return true;
#else
			error_code ec;
			// Write the response
			std::size_t bytes_sent = http::write(stream, const_cast<msg_type&>(data()), ec);
			set_last_error(ec);
			callback(ec, bytes_sent);
			return (!bool(ec));
#endif
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_HTTP_SEND_OP_HPP__

#endif
