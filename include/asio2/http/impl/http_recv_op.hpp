/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#include <asio2/http/detail/http_util.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class http_recv_op
	{
	public:
		using body_type   = typename args_t::body_t;
		using buffer_type = typename args_t::buffer_t;

		/**
		 * @constructor
		 */
		http_recv_op() noexcept {}

		/**
		 * @destructor
		 */
		~http_recv_op() = default;

	protected:
		template<typename MatchCondition>
		void _http_post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!derive.is_started())
			{
				if (derive.state() == state_t::started)
				{
					derive._do_disconnect(asio2::get_last_error(), std::move(this_ptr));
				}
				return;
			}

			try
			{
				if constexpr (args_t::is_session)
				{
					if (derive.is_http())
					{
						// Make the request empty before reading,
						// otherwise the operation behavior is undefined.
						derive.req_.reset();

						// Read a request
						http::async_read(derive.stream(), derive.buffer().base(), derive.req_,
							make_allocator(derive.rallocator(),
								[&derive, self_ptr = std::move(this_ptr), condition = std::move(condition)]
						(const error_code & ec, std::size_t bytes_recvd) mutable
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), std::move(condition));
						}));
					}
					else
					{
						// Read a message into our buffer
						derive.ws_stream().async_read(derive.buffer().base(),
							make_allocator(derive.rallocator(),
								[&derive, self_ptr = std::move(this_ptr), condition = std::move(condition)]
						(const error_code & ec, std::size_t bytes_recvd) mutable
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), std::move(condition));
						}));
					}
				}
				else
				{
					// Make the request empty before reading,
					// otherwise the operation behavior is undefined.
					derive.rep_.reset();

					// Receive the HTTP response
					http::async_read(derive.stream(), derive.buffer().base(), derive.rep_,
						make_allocator(derive.rallocator(),
							[&derive, self_ptr = std::move(this_ptr), condition = std::move(condition)]
					(const error_code & ec, std::size_t bytes_recvd) mutable
					{
						derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
					}));
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);

				derive._do_disconnect(e.code(), derive.selfptr());
			}
		}

		template<typename MatchCondition>
		void _http_handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			std::ignore = bytes_recvd;

			if (!derive.is_started())
			{
				if (derive.state() == state_t::started)
				{
					derive._do_disconnect(ec, std::move(this_ptr));
				}
				return;
			}

			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				if constexpr (args_t::is_session)
				{
					if (derive.is_http())
					{
						std::string_view target = derive.req_.target();
						http::parses::http_parser_parse_url(
							target.data(), target.size(), 0, std::addressof(derive.req_.url_.parser()));

						derive.rep_.result(http::status::unknown);
						derive.rep_.keep_alive(derive.req_.keep_alive());

						if (derive._check_upgrade(this_ptr, condition))
							return;

						derive._fire_recv(this_ptr, condition);

						// note : can't read write the variable of "req_" after _fire_recv, it maybe
						// cause crash, eg :
						// user called response.defer() in the recv callback, and pass the defer_ptr
						// into another thread, then code run to here, at this time, the "req_" maybe
						// read write in two thread : this thread and "another thread".
						// note : can't call "_do_disconnect" at here, beacuse if user has called
						// response.defer() in the recv callback, this session maybe closed before
						// the response is sent to the client.
						//if (derive.req_.need_eof() || !derive.req_.keep_alive())
						//{
						//	derive._do_disconnect(asio::error::operation_aborted, derive.selfptr());
						//	return;
						//}
					}
					else
					{
						derive.req_.ws_frame_type_ = websocket::frame::message;
						derive.req_.ws_frame_data_ = { reinterpret_cast<std::string_view::const_pointer>(
							derive.buffer().data().data()), bytes_recvd };

						derive._fire_recv(this_ptr, condition);

						derive.buffer().consume(derive.buffer().size());

						derive._post_recv(std::move(this_ptr), std::move(condition));
					}
				}
				else
				{
					derive._fire_recv(this_ptr, condition);

					derive._post_recv(std::move(this_ptr), std::move(condition));
				}
			}
			else
			{
				// This means they closed the connection
				//if (ec == http::error::end_of_stream)
				derive._do_disconnect(ec, derive.selfptr());
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

	protected:
	};
}

#endif // !__ASIO2_HTTP_RECV_OP_HPP__
