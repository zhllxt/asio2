/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

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
			if (!derive.is_started())
				return;

			try
			{
				if constexpr (isSession)
				{
					if (derive.is_http())
					{
						// Make the request empty before reading,
						// otherwise the operation behavior is undefined.
						derive.req_.reset();

						// Read a request
						http::async_read(derive.stream(), derive.buffer().base(), derive.req_,
							asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
								[this, self_ptr = std::move(this_ptr), condition]
						(const error_code & ec, std::size_t bytes_recvd) mutable
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
						})));
					}
					else
					{
						// Read a message into our buffer
						derive.ws_stream().async_read(derive.buffer().base(),
							asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
								[this, self_ptr = std::move(this_ptr), condition]
						(const error_code & ec, std::size_t bytes_recvd) mutable
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
						})));
					}
				}
				else
				{
					// Make the request empty before reading,
					// otherwise the operation behavior is undefined.
					derive.rep_.reset();

					// Receive the HTTP response
					http::async_read(derive.stream(), derive.buffer().base(), derive.rep_,
						asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
							[this, self_ptr = std::move(this_ptr), condition]
					(const error_code & ec, std::size_t bytes_recvd) mutable
					{
						derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
					})));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._do_disconnect(e.code());
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
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				if constexpr (isSession)
				{
					if (derive._check_upgrade(this_ptr, condition))
						return;

					if (derive.is_http())
					{
						derive.rep_.result(http::status::unknown);
						derive.rep_.keep_alive(derive.req_.keep_alive());
					}
					else
					{
						derive.req_.ws_frame_type_ = websocket::frame::message;
						derive.req_.ws_frame_data_ = { reinterpret_cast<std::string_view::const_pointer>(
							derive.buffer().data().data()), bytes_recvd };
					}

					derive._fire_recv(this_ptr, condition);

					if (derive.is_http())
					{
						if (derive.req_.need_eof() || !derive.req_.keep_alive())
						{
							derive._do_disconnect(asio::error::operation_aborted);
							return;
						}
					}
					else
					{
						derive.buffer().consume(derive.buffer().size());

						derive._post_recv(std::move(this_ptr), condition);
					}
				}
				else
				{
					derive._fire_recv(this_ptr, condition);

					derive._post_recv(std::move(this_ptr), condition);
				}
			}
			else
			{
				// This means they closed the connection
				//if (ec == http::error::end_of_stream)
				derive._do_disconnect(ec);
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
