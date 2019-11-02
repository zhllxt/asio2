/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_HTTP_RECV_OP_HPP__
#define __ASIO2_HTTP_RECV_OP_HPP__

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
	class http_recv_op
	{
	public:
		/**
		 * @constructor
		 */
		http_recv_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~http_recv_op() = default;

	protected:
		template<typename MatchCondition>
		void _http_post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (derive.is_started())
			{
				try
				{
					if constexpr (isSession)
					{
						// Make the request empty before reading,
						// otherwise the operation behavior is undefined.
						derive.req_ = {};

						// Read a request
						http::async_read(derive.stream(), derive.buffer().base(), derive.req_,
							asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
								[this, self_ptr = std::move(this_ptr), condition](const error_code & ec, std::size_t bytes_recvd)
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
						})));
					}
					else
					{
						// Make the request empty before reading,
						// otherwise the operation behavior is undefined.
						derive.rep_ = {};

						// Receive the HTTP response
						http::async_read(derive.stream(), derive.buffer().base(), derive.rep_,
							asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
								[this, self_ptr = std::move(this_ptr), condition](const error_code & ec, std::size_t bytes_recvd)
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
						})));
					}
				}
				catch (system_error & e)
				{
					set_last_error(e);

					derive._do_stop(e.code());
				}
			}
		}

		template<typename MatchCondition>
		void _http_handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			set_last_error(ec);

			std::ignore = bytes_recvd;

			if (!ec)
			{
				// every times recv data,we update the last active time.
				derive.reset_active_time();

				if constexpr (isSession)
				{
					derive._fire_recv(this_ptr, derive.req_);
					if (derive.req_.need_eof() || !derive.req_.keep_alive())
					{
						derive._do_stop(asio::error::operation_aborted);
						return;
					}
				}
				else
				{
					derive._fire_recv(this_ptr, derive.rep_);
				}

				derive._post_recv(std::move(this_ptr), condition);
			}
			else
			{
				// This means they closed the connection
				//if (ec == http::error::end_of_stream)
				derive._do_stop(ec);
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_HTTP_RECV_OP_HPP__

#endif
