/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

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
	template<class, class = std::void_t<>>
	struct is_websocket_client : std::false_type {};

	template<class T>
	struct is_websocket_client<T, std::void_t<decltype(std::declval<T&>().ws_stream())>> : std::true_type {};

	template<class, class = std::void_t<>>
	struct is_websocket_server : std::false_type {};

	template<class T>
	struct is_websocket_server<T, std::void_t<decltype(std::declval<typename T::session_type&>().
		ws_stream())>> : std::true_type {};

	template<class derived_t, class args_t>
	class ws_stream_cp
	{
	public:
		using stream_type = typename args_t::stream_t;

		/**
		 * @constructor
		 */
		ws_stream_cp() {}

		/**
		 * @destructor
		 */
		~ws_stream_cp() = default;

		inline stream_type & ws_stream()
		{
			ASIO2_ASSERT(bool(this->ws_stream_));
			return (*(this->ws_stream_));
		}

	protected:
		template<typename MatchCondition, typename Socket>
		inline void _ws_init(const condition_wrap<MatchCondition>& condition, Socket& socket)
		{
			detail::ignore_unused(condition, socket);

			this->ws_stream_ = std::make_unique<stream_type>(socket);

			// Set suggested timeout settings for the websocket
			if constexpr (args_t::is_session)
			{
				this->ws_stream_->set_option(websocket::stream_base::timeout::suggested(
					beast::role_type::server));
			}
			else
			{
				this->ws_stream_->set_option(websocket::stream_base::timeout::suggested(
					beast::role_type::client));
			}
		}

		template<typename MatchCondition, typename Socket>
		inline void _ws_start(
			const std::shared_ptr<derived_t>& this_ptr,
			const condition_wrap<MatchCondition>& condition, Socket& socket)
		{
			detail::ignore_unused(this_ptr, condition, socket);
		}

		template<typename Fn>
		inline void _ws_stop(std::shared_ptr<derived_t> this_ptr, Fn&& fn)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!this->ws_stream_)
			{
				(fn)();
				return;
			}

			// bug fixed : resolve beast::websocket::detail::soft_mutex 
			// BEAST_ASSERT(id_ != T::id); failed (line 89).
			// 
			// If this assert goes off it means you are attempting to
			// simultaneously initiate more than one of same asynchronous
			// operation, which is not allowed. For example, you must wait
			// for an async_read to complete before performing another
			// async_read.
			//
			// don't use "this_ptr = std::move(this_ptr)" in lambda capture below.
			auto task = [this, &derive, this_ptr, fn = std::forward<Fn>(fn)]
			(event_queue_guard<derived_t>&& g) mutable
			{
				// Set the timeout to none to cancel the websocket timeout timer, otherwise
				// we'll have to wait a lot of seconds util the timer is timeout.
				websocket::stream_base::timeout opt{};
				opt.handshake_timeout = websocket::stream_base::none();
				opt.idle_timeout      = websocket::stream_base::none();
				this->ws_stream_->set_option(opt);

				// Can't call close twice
				// TODO return a custom error code
				// BEAST_ASSERT(! impl.wr_close);
				if (!this->ws_stream_->is_open())
				{
					// must Reset the control frame callback. the control frame callback hold the 
					// self shared_ptr, if don't reset it, will cause memory leaks.
					this->ws_stream_->control_callback();

					(fn)();

					return;
				}

				// Close the WebSocket connection
				this->ws_stream_->async_close(websocket::close_code::normal,
					asio::bind_executor(derive.io().strand(),
						[this, this_ptr = std::move(this_ptr), g = std::move(g), f = std::move(fn)]
				(error_code ec) mutable
				{
					//if (ec)
					//	return;

					// If we get here then the connection is closed gracefully

					// must Reset the control frame callback. the control frame callback hold the 
					// self shared_ptr, if don't reset it, will cause memory leaks.
					this->ws_stream_->control_callback();

					(f)();
				}));
			};

			derive.push_event([&derive, t = std::move(task)](event_queue_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				derive.post(std::move(task));
				return true;
			});
		}

		template<typename MatchCondition>
		inline void _ws_post_recv(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if (derive.is_started())
			{
				try
				{
					ASIO2_ASSERT(bool(this->ws_stream_));
					// Read a message into our buffer
					this->ws_stream_->async_read(derive.buffer().base(),
						asio::bind_executor(derive.io().strand(),
							make_allocator(derive.rallocator(),
								[&derive, this_ptr = std::move(this_ptr), condition = std::move(condition)]
					(const error_code & ec, std::size_t bytes_recvd) mutable
					{
						derive._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(condition));
					})));
				}
				catch (system_error & e)
				{
					set_last_error(e);

					derive._do_disconnect(e.code());
				}
			}
		}

		template<typename MatchCondition>
		void _ws_handle_recv(const error_code & ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			// bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.
			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				derive._fire_recv(this_ptr, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(derive.buffer().data().data()), bytes_recvd), condition);

				derive.buffer().consume(bytes_recvd);

				derive._post_recv(std::move(this_ptr), std::move(condition));
			}
			else
			{
				derive._do_disconnect(ec);
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		template<typename MatchCondition>
		inline void _post_control_callback(std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));
			// Set the control callback. This will be called
			// on every incoming ping, pong, and close frame.
			// can't use push_event, just only use asio::post, beacuse the callback handler
			// will not be called immediately, it is called only when it receives an event 
			// on the another side.
			derive.post([this, &derive, this_ptr = std::move(this_ptr), condition]() mutable
			{
				this->ws_stream_->control_callback(asio::bind_executor(derive.io().strand(),
					[&derive, this_ptr = std::move(this_ptr), condition]
				(websocket::frame_type kind, beast::string_view payload) mutable
				{
					// bug fixed : can't use "std::move(this_ptr)" below, otherwise 
					// when enter this lambda next time, the "this_ptr" is nullptr.
					derive._handle_control_callback(kind, payload, this_ptr, condition);
				}));
			});
		}

		template<typename MatchCondition>
		inline void _handle_control_callback(websocket::frame_type kind, beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(payload, this_ptr, condition);

			// Note that the connection is alive
			derive.update_alive_time();

			switch (kind)
			{
			case websocket::frame_type::ping:
				derive._handle_control_ping(payload, std::move(this_ptr), std::move(condition));
				break;
			case websocket::frame_type::pong:
				derive._handle_control_pong(payload, std::move(this_ptr), std::move(condition));
				break;
			case websocket::frame_type::close:
				derive._handle_control_close(payload, std::move(this_ptr), std::move(condition));
				break;
			default:break;
			}
		}

		template<typename MatchCondition>
		inline void _handle_control_ping(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore_unused(payload, this_ptr, condition);
		}

		template<typename MatchCondition>
		inline void _handle_control_pong(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			detail::ignore_unused(payload, this_ptr, condition);
		}

		template<typename MatchCondition>
		inline void _handle_control_close(beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(payload, this_ptr, condition);

			derive._do_disconnect(websocket::error::closed);
		}

		template<typename MatchCondition, typename Response, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _post_upgrade(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition,
			Response& rep)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

			auto task = [this, &derive, this_ptr, condition, &rep](event_queue_guard<derived_t>&& g) mutable
			{
				// Perform the websocket handshake
				this->ws_stream_->async_handshake(rep, derive.host_, derive.upgrade_target(),
					asio::bind_executor(derive.io().strand(), make_allocator(derive.rallocator(),
						[&derive, this_ptr = std::move(this_ptr), condition = std::move(condition), g = std::move(g)]
				(error_code const& ec) mutable
				{
					derive._handle_upgrade(ec, std::move(this_ptr), std::move(condition));
				})));
			};

			derive.push_event([&derive, t = std::move(task)](event_queue_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				derive.post(std::move(task));
				return true;
			});
		}

		template<typename MatchCondition, typename Request, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _post_upgrade(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition,
			Request const& req)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

			auto task = [this, &derive, this_ptr, condition, &req](event_queue_guard<derived_t>&& g) mutable
			{
				// Accept the websocket handshake
				this->ws_stream_->async_accept(req, asio::bind_executor(derive.io().strand(),
					make_allocator(derive.rallocator(),
						[&derive, this_ptr = std::move(this_ptr), condition = std::move(condition), g = std::move(g)]
				(error_code ec) mutable
				{
					derive._handle_upgrade(ec, std::move(this_ptr), std::move(condition));
				})));
			};

			derive.push_event([&derive, t = std::move(task)](event_queue_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				derive.post(std::move(task));
				return true;
			});
		}

		template<typename MatchCondition, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _post_upgrade(std::shared_ptr<derived_t> this_ptr, condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

			auto task = [this, &derive, this_ptr, condition](event_queue_guard<derived_t>&& g) mutable
			{
				// Accept the websocket handshake
				this->ws_stream_->async_accept(asio::bind_executor(derive.io().strand(),
					make_allocator(derive.rallocator(),
						[&derive, this_ptr = std::move(this_ptr), condition = std::move(condition), g = std::move(g)]
				(error_code ec) mutable
				{
					derive._handle_upgrade(ec, std::move(this_ptr), std::move(condition));
				})));
			};

			derive.push_event([&derive, t = std::move(task)](event_queue_guard<derived_t>&& g) mutable
			{
				auto task = [g = std::move(g), t = std::move(t)]() mutable
				{
					t(std::move(g));
				};
				derive.post(std::move(task));
				return true;
			});
		}

		template<typename MatchCondition>
		inline void _handle_upgrade(const error_code & ec, std::shared_ptr<derived_t> this_ptr,
			condition_wrap<MatchCondition> condition)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

			if constexpr (args_t::is_session)
			{
				// Use "sessions().post" to ensure that the _fire_accept function and the _fire_upgrade
				// function are fired in the same thread
				derive.sessions().post([&derive, ec, this_ptr = std::move(this_ptr), condition]() mutable
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

						derive._do_disconnect(e.code());
					}
				});
			}
			else
			{
				set_last_error(ec);

				derive._fire_upgrade(this_ptr, ec);

				derive._done_connect(ec, std::move(this_ptr), std::move(condition));
			}
		}

	protected:
		std::unique_ptr<stream_type>   ws_stream_;
	};
}

#endif // !__ASIO2_WS_STREAM_COMPONENT_HPP__
