/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

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
		 * @brief constructor
		 */
		ws_stream_cp() {}

		/**
		 * @brief destructor
		 */
		~ws_stream_cp() noexcept {}

		/**
		 * @brief get the websocket stream object refrence
		 */
		inline stream_type & ws_stream() noexcept
		{
			ASIO2_ASSERT(bool(this->ws_stream_));
			return (*(this->ws_stream_));
		}

	protected:
		template<typename C, typename Socket>
		inline void _ws_init(std::shared_ptr<ecs_t<C>>& ecs, Socket& socket)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, ecs, socket);

			// In previous versions, "_ws_init" maybe called multi times, when "_ws_init" was called,
			// the "_ws_stop" maybe called at the same time, but "_ws_init" and "_ws_stop" was called
			// at different threads, this will cause the diffrent threads "read, write" the "ws_stream_"
			// variable, and then cause crash.

			if constexpr (args_t::is_client)
			{
				ASIO2_ASSERT(derive.io().running_in_this_thread());
			}
			else
			{
				ASIO2_ASSERT(derive.sessions().io().running_in_this_thread());
			}

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

		template<typename C, typename Socket>
		inline void _ws_start(std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, Socket& socket)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(derive, this_ptr, ecs, socket);

			ASIO2_ASSERT(derive.io().running_in_this_thread());
		}

		template<typename DeferEvent>
		inline void _ws_stop(std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			if (!this->ws_stream_)
				return;

			// bug fixed : resolve beast::websocket::detail::soft_mutex 
			// BHO_ASSERT(id_ != T::id); failed (line 89).
			// 
			// If this assert goes off it means you are attempting to
			// simultaneously initiate more than one of same asynchronous
			// operation, which is not allowed. For example, you must wait
			// for an async_read to complete before performing another
			// async_read.
			derive.disp_event([this, &derive, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				// must construct a new chain
				defer_event chain(std::move(e), std::move(g));

				// Set the handshake timeout to a small value, otherwise if the remote don't
				// send a websocket close frame, the async_close's callback will never be
				// called.
				websocket::stream_base::timeout opt{};
				opt.handshake_timeout = std::chrono::milliseconds(ws_shutdown_timeout);
				opt.idle_timeout      = websocket::stream_base::none();

				try
				{
					derive.ws_stream_->set_option(opt);
				}
				catch (system_error const& e)
				{
					set_last_error(e);
				}

				// Can't call close twice
				// TODO return a custom error code
				// BHO_ASSERT(! impl.wr_close);
				if (!this->ws_stream_->is_open())
				{
					// must Reset the control frame callback. the control frame callback hold the 
					// self shared_ptr, if don't reset it, will cause memory leaks.
					this->ws_stream_->control_callback();

					return;
				}

			#if defined(_DEBUG) || defined(DEBUG)
				ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
				derive.post_send_counter_++;
			#endif

				// Close the WebSocket connection
				// async_close behavior : 
				// send a websocket close frame to the remote, and wait for recv a websocket close
				// frame from the remote.
				// async_close maybe close the socket directly.
				this->ws_stream_->async_close(websocket::close_code::normal,
				[&derive, this_ptr = std::move(this_ptr), chain = std::move(chain)]
				(error_code ec) mutable
				{
				#if defined(_DEBUG) || defined(DEBUG)
					derive.post_send_counter_--;
				#endif

					detail::ignore_unused(derive, ec);

					//if (ec)
					//	return;

					// If we get here then the connection is closed gracefully

					// must Reset the control frame callback. the control frame callback hold the 
					// self shared_ptr, if don't reset it, will cause memory leaks.
					derive.ws_stream_->control_callback();
				});
			}, chain.move_guard());
		}

		template<typename C>
		inline void _ws_post_recv(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
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

			ASIO2_ASSERT(bool(this->ws_stream_));

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
			derive.post_recv_counter_++;
		#endif

			// Read a message into our buffer
			this->ws_stream_->async_read(derive.buffer().base(), make_allocator(derive.rallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
			(const error_code& ec, std::size_t bytes_recvd) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_recv_counter_--;
			#endif

				derive._handle_recv(ec, bytes_recvd, std::move(this_ptr), std::move(ecs));
			}));
		}

		template<typename C>
		void _ws_handle_recv(
			const error_code& ec, std::size_t bytes_recvd,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io().running_in_this_thread());

			set_last_error(ec);

			if (!derive.is_started())
			{
				if (derive.state() == state_t::started)
				{
					derive._do_disconnect(ec, std::move(this_ptr));
				}
				return;
			}

			// bytes_recvd : The number of bytes in the streambuf's get area up to and including the delimiter.
			if (!ec)
			{
				// every times recv data,we update the last alive time.
				derive.update_alive_time();

				derive._fire_recv(this_ptr, ecs, std::string_view(reinterpret_cast<
					std::string_view::const_pointer>(derive.buffer().data().data()), bytes_recvd));

				derive.buffer().consume(bytes_recvd);

				derive._post_recv(std::move(this_ptr), std::move(ecs));
			}
			else
			{
				derive._do_disconnect(ec, std::move(this_ptr));
			}
			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		template<typename C>
		inline void _post_control_callback(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));
			// Set the control callback. This will be called
			// on every incoming ping, pong, and close frame.
			// can't use push_event, just only use asio::post, beacuse the callback handler
			// will not be called immediately, it is called only when it receives an event 
			// on the another side.
			asio::post(derive.io().context(), make_allocator(derive.wallocator(),
			[this, &derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]() mutable
			{
				this->ws_stream_->control_callback(
				[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs)]
				(websocket::frame_type kind, beast::string_view payload) mutable
				{
					// bug fixed : can't use "std::move(this_ptr)" below, otherwise 
					// when enter this lambda next time, the "this_ptr" is nullptr.
					derive._handle_control_callback(kind, payload, this_ptr, ecs);
				});
			}));
		}

		template<typename C>
		inline void _handle_control_callback(
			websocket::frame_type kind, beast::string_view payload,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(payload, this_ptr, ecs);

			// Note that the connection is alive
			derive.update_alive_time();

			switch (kind)
			{
			case websocket::frame_type::ping:
				derive._handle_control_ping(payload, std::move(this_ptr), std::move(ecs));
				break;
			case websocket::frame_type::pong:
				derive._handle_control_pong(payload, std::move(this_ptr), std::move(ecs));
				break;
			case websocket::frame_type::close:
				derive._handle_control_close(payload, std::move(this_ptr), std::move(ecs));
				break;
			default:break;
			}
		}

		template<typename C>
		inline void _handle_control_ping(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(payload, this_ptr, ecs);
		}

		template<typename C>
		inline void _handle_control_pong(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			detail::ignore_unused(payload, this_ptr, ecs);
		}

		template<typename C>
		inline void _handle_control_close(
			beast::string_view payload, std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			detail::ignore_unused(payload, this_ptr, ecs);

			if (derive.state() == state_t::started)
			{
				derive._do_disconnect(websocket::error::closed, std::move(this_ptr));
			}
		}

		template<typename C, typename DeferEvent, typename Response, bool IsSession = args_t::is_session>
		typename std::enable_if_t<!IsSession, void>
		inline _post_upgrade(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, Response& rep, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			// Perform the websocket handshake
			this->ws_stream_->async_handshake(rep, derive.host_, derive.get_upgrade_target(),
				make_allocator(derive.wallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			(error_code const& ec) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				derive._handle_upgrade(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent, typename Request, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _post_upgrade(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, Request const& req, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_send_counter_.load() == 0);
			derive.post_send_counter_++;
		#endif

			// Accept the websocket handshake
			// just write response.
			this->ws_stream_->async_accept(req, make_allocator(derive.wallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			(error_code ec) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_send_counter_--;
			#endif

				derive._handle_upgrade(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent, bool IsSession = args_t::is_session>
		typename std::enable_if_t<IsSession, void>
		inline _post_upgrade(std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

		#if defined(_DEBUG) || defined(DEBUG)
			ASIO2_ASSERT(derive.post_recv_counter_.load() == 0);
			derive.post_recv_counter_++;
		#endif

			// Accept the websocket handshake
			// first read request, then write response.
			this->ws_stream_->async_accept(make_allocator(derive.wallocator(),
			[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			(error_code ec) mutable
			{
			#if defined(_DEBUG) || defined(DEBUG)
				derive.post_recv_counter_--;
			#endif

				derive._handle_upgrade(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}));
		}

		template<typename C, typename DeferEvent>
		inline void _session_handle_upgrade(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// Use "sessions().dispatch" to ensure that the _fire_accept function and the _fire_upgrade
			// function are fired in the same thread
			derive.sessions().dispatch(
			[&derive, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				set_last_error(ec);

				derive._fire_upgrade(this_ptr);

				if (ec)
				{
					derive._do_disconnect(ec, std::move(this_ptr), std::move(chain));

					return;
				}

				derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			});
		}

		template<typename C, typename DeferEvent>
		inline void _client_handle_upgrade(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			set_last_error(ec);

			derive._fire_upgrade(this_ptr);

			derive._done_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
		}

		template<typename C, typename DeferEvent>
		inline void _handle_upgrade(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(bool(this->ws_stream_));

			if constexpr (args_t::is_session)
			{
				derive._session_handle_upgrade(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				derive._client_handle_upgrade(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

	protected:
		std::unique_ptr<stream_type>   ws_stream_;
	};
}

#endif // !__ASIO2_WS_STREAM_COMPONENT_HPP__
