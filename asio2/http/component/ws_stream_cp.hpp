/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef ASIO_STANDALONE

#ifndef __ASIO2_WS_STREAM_COMPONENT_HPP__
#define __ASIO2_WS_STREAM_COMPONENT_HPP__

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
	template<class derived_t, class stream_t, bool isSession>
	class ws_stream_cp
	{
	public:
		using stream_type = stream_t;

		/**
		 * @constructor
		 */
		template<typename Arg>
		ws_stream_cp(Arg & arg) : derive(static_cast<derived_t&>(*this)), ws_stream_(arg)
		{
		}

		/**
		 * @destructor
		 */
		~ws_stream_cp() = default;

		inline stream_t & ws_stream() { return this->ws_stream_; }

	protected:
		template<typename MatchCondition>
		inline void _ws_start(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
		}

		template<typename Fn>
		inline void _ws_stop(std::shared_ptr<derived_t> this_ptr, Fn&& fn)
		{
			// Close the WebSocket connection
			this->ws_stream_.async_close(websocket::close_code::normal,
				asio::bind_executor(derive.io().strand(),
					[this, self_ptr = std::move(this_ptr), f = std::forward<Fn>(fn)](error_code ec)
			{
				//if (ec)
				//	return;

				// If we get here then the connection is closed gracefully

				// must Reset the control frame callback. the control frame callback hold the self shared_ptr,
				// if don't reset it, will cause memory leaks.
				this->ws_stream_.control_callback();

				(f)();
			}));
		}

		template<typename MatchCondition>
		inline void _ws_post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if (derive.is_started())
			{
				try
				{
					if constexpr (std::is_same_v<MatchCondition, void>)
					{
						// Read a message into our buffer
						this->ws_stream_.async_read(derive.buffer().base(),
							asio::bind_executor(derive.io().strand(),
								make_allocator(derive.rallocator(),
									[this, self_ptr = std::move(this_ptr), condition]
						(const error_code & ec, std::size_t bytes_recvd)
						{
							derive._handle_recv(ec, bytes_recvd, std::move(self_ptr), condition);
						})));
					}
					else
					{
						std::ignore;
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
		inline void _post_control_callback(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			// Set the control callback. This will be called
			// on every incoming ping, pong, and close frame.
			this->ws_stream_.control_callback(asio::bind_executor(derive.io().strand(),
				[this, self_ptr = std::move(this_ptr), condition]
			(websocket::frame_type kind, beast::string_view payload)
			{
				derive._handle_control_callback(kind, payload, std::move(self_ptr), condition);
			}));
		}

		template<typename MatchCondition>
		void _handle_control_callback(websocket::frame_type kind, beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			std::ignore = payload;
			std::ignore = this_ptr;
			std::ignore = condition;

			if (kind == websocket::frame_type::close)
			{
				derive._do_stop(websocket::error::closed);
				return;
			}

			// Note that the connection is alive
			derive.reset_active_time();

			// Note that there is activity
			// Note that the connection is alive
			//ping_state_ = 0;

			// Set the timer
			//timer_.expires_after(std::chrono::seconds(15));
		}

		template<typename MatchCondition>
		inline void _post_upgrade(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			if constexpr (isSession)
			{
				// Accept the websocket handshake
				this->ws_stream_.async_accept(asio::bind_executor(derive.io().strand(),
					make_allocator(derive.rallocator(),
						[this, self_ptr = std::move(this_ptr), condition](error_code ec)
				{
					derive._handle_upgrade(ec, std::move(self_ptr), condition);
				})));
			}
			else
			{
				// Perform the websocket handshake
				this->ws_stream_.async_handshake(derive.upgrade_rep_, derive.host_, "/",
					asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
						[this, self_ptr = std::move(this_ptr), condition](error_code const& ec)
				{
					derive._handle_upgrade(ec, std::move(self_ptr), condition);
				})));
			}
		}

		template<typename MatchCondition, typename Request>
		inline void _post_upgrade(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition,
			Request const& req)
		{
			static_assert(isSession, "This function must be invoked on the server side.");
			// Accept the websocket handshake
			this->ws_stream_.async_accept(req, asio::bind_executor(derive.io().strand(),
				make_allocator(derive.rallocator(),
					[this, self_ptr = std::move(this_ptr), condition](error_code ec)
			{
				derive._handle_upgrade(ec, std::move(self_ptr), condition);
			})));
		}

		template<typename MatchCondition>
		inline void _handle_upgrade(const error_code & ec, std::shared_ptr<derived_t> self_ptr, condition_wrap<MatchCondition> condition)
		{
			this->ws_stream_.binary(true); // Setting the message type to binary.

			if constexpr (isSession)
			{
				derive.sessions().post([this, ec, this_ptr = std::move(self_ptr), condition]() mutable
				{
					try
					{
						set_last_error(ec);

						derive._fire_upgrade(this_ptr, ec);

						asio::detail::throw_error(ec);

						derive._done_connect(ec, std::move(this_ptr), std::move(condition));
					}
					catch (system_error & e)
					{
						set_last_error(e);
						derive._do_stop(e.code());
					}
				});
			}
			else
			{
				set_last_error(ec);

				derive._fire_upgrade(self_ptr, ec);

				derive._done_connect(ec, std::move(self_ptr), std::move(condition));
			}
		}

	protected:
		derived_t      & derive;

		stream_t         ws_stream_;
	};
}

#endif // !__ASIO2_WS_STREAM_COMPONENT_HPP__

#endif
