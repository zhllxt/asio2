/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SHUTDOWN_COMPONENT_HPP__
#define __ASIO2_SHUTDOWN_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/iopool.hpp>
#include <asio2/base/listener.hpp>

#include <asio2/base/impl/event_queue_cp.hpp>

// Asio end socket functions: cancel, shutdown, close, release :
// https://stackoverflow.com/questions/51468848/asio-end-socket-functions-cancel-shutdown-close-release
// The proper steps are:
// 1.Call shutdown() to indicate that you will not write any more data to the socket.
// 2.Continue to (async-) read from the socket until you get either an error or the connection is closed.
// 3.Now close() the socket (in the async read handler).
// If you don't do this, you may end up closing the connection while the other side is still sending data.
// This will result in an ungraceful close.

// http://www.purecpp.cn/detail?id=2303

// https://learn.microsoft.com/en-us/windows/win32/api/winsock/nf-winsock-shutdown
// 
// To assure that all data is sent and received on a connected socket before it is closed,
// an application should use shutdown to close connection before calling closesocket. 
// One method to wait for notification that the remote end has sent all its data and initiated 
// a graceful disconnect uses the WSAEventSelect function as follows :
// 
// 1. Call WSAEventSelect to register for FD_CLOSE notification.
// 2. Call shutdown with how=SD_SEND.
// 3. When FD_CLOSE received, call the recv or WSARecv until the function completes with success
//    and indicates that zero bytes were received. If SOCKET_ERROR is returned, then the graceful
//    disconnect is not possible.
// 4. Call closesocket.
// 
// Another method to wait for notification that the remote end has sent all its data and initiated 
// a graceful disconnect uses overlapped receive calls follows :
// 
// 1. Call shutdown with how=SD_SEND.
// 2. Call recv or WSARecv until the function completes with success and indicates zero bytes were
//    received. If SOCKET_ERROR is returned, then the graceful disconnect is not possible.
// 3. Call closesocket.

// https://stackoverflow.com/questions/60105082/using-shutdown-for-a-udp-socket
// Calling shutdown() on a UDP socket does nothing on the wire, and only affects the state of the
// socket object.
// after test on windows, call shutdown on udp socket, the recv won't return with a error.

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class shutdown_cp
	{
	public:
		using self = shutdown_cp<derived_t, args_t>;

	public:
		/**
		 * @brief constructor
		 */
		shutdown_cp() noexcept {}

		/**
		 * @brief destructor
		 */
		~shutdown_cp() = default;

	protected:
		template<typename DeferEvent>
		inline void _do_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// call shutdown manual in client:
			// 1. the session recv function will return with error of eof,
			//    call closesocket in the session recv function is ok.
			// 2. the client  recv function will return with error of eof
			//    call closesocket in the client  recv function is ok.

			// but at ssl mode:
			// 1. we must call async_shutdown to execute the ssl stream close handshake first.
			// 2. then we can call shutdown.

			// when the recv function is returned with error of eof, there are two possible that
			// can trigger this situation:
			// 1. call shutdown manual.
			// 2. recvd the shutdown notify of the another peer.
			// so:
			// 1. when we call shutdown manual, then the recv function will return with error eof,
			//    at this time, we shouldn't call shutdown and make shutdown timer again, we should
			//    call closesocket directly.
			// 2. when we recvd the shutdown notify of the another peer, then the recv function will
			//    return with error eof, at this time, we shouldn't call shutdown and make shutdown
			//    timer again too, we should call closesocket directly too. 
			// so:
			//    we should check whether the shutdown timer is empty, if it is true, it means the
			//    shutdown is not yet called by manual, then we should call shutdown. if it is false,
			//    it means the shutdown is called already by manual, then we should call closesocket.

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("shutdown_cp::_do_shutdown enter: {} {}", ec.value(), ec.message());

			// if disconnect is in progress, and not finished, we can't do a new disconnect again.
			// otherwise, this situation will has problem:
			// client.post([&]()
			// {
			//		client.stop();
			//		client.start(...);
			// });
			// after post a connect event, then a new disconnect event maybe pushed into the queue
			// again, this will cause the start has no effect.
			if (derive.shutdown_timer_ || derive.disconnecting_)
			{
				derive._stop_shutdown_timer(std::move(this_ptr));

				return;
			}

			derive.disp_event(
			[&derive, ec, this_ptr = std::move(this_ptr), e = chain.move_event()]
			(event_queue_guard<derived_t> g) mutable
			{
				set_last_error(ec);

				// make a new chain, if the chain can't be passed to other functions, the chain
				// should be destroyed at the end of this function, then the function of this
				// chain will be called, otherwise this situation will be happen:
				// 
				// client.post([&]()
				// {
				//		client.stop();
				//		client.start(...);
				// });
				// 
				// client stop will post a event, and this event is before the client.start, and
				// at this time the state maybe stopped, so the chain can't be passed to other
				// fuctions, then the event guard will destroyed, then the next event "client.start"
				// will be called, then the stop's callback "do stop" will be called, but at this
				// time, the state is starting, which not equal to stopped.
				// 
				defer_event chain(std::move(e), std::move(g));

				ASIO2_LOG_DEBUG("shutdown_cp::_do_shutdown leave: {} {} state={}",
					ec.value(), ec.message(), detail::to_string(derive.state_.load()));

				state_t expected = state_t::started;
				if (derive.state_.compare_exchange_strong(expected, state_t::started))
				{
					derive.disconnecting_ = true;

					return derive._post_shutdown(ec, std::move(this_ptr), std::move(chain));
				}

				expected = state_t::starting;
				if (derive.state_.compare_exchange_strong(expected, state_t::starting))
				{
					derive.disconnecting_ = true;

					return derive._post_shutdown(ec, std::move(this_ptr), std::move(chain));
				}
			}, chain.move_guard());
		}

		template<typename DeferEvent>
		inline void _post_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("shutdown_cp::_post_shutdown: {} {}", ec.value(), ec.message());

			if constexpr (std::is_same_v<typename derived_t::socket_type::protocol_type, asio::ip::tcp>)
			{
				derive._post_shutdown_tcp(ec, std::move(this_ptr), std::move(chain));
			}
			else
			{
				derive._handle_shutdown(ec, std::move(this_ptr), std::move(chain));
			}
		}

		template<typename DeferEvent>
		inline void _post_shutdown_tcp(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// the socket maybe closed already in the connect timeout timer.
			if (derive.socket().is_open())
			{
				error_code ec_linger{};

				asio::socket_base::linger lnger{};

				derive.socket().lowest_layer().get_option(lnger, ec_linger);

				// call socket's close function to notify the _handle_recv function response with 
				// error > 0 ,then the socket can get notify to exit
				// Call shutdown() to indicate that you will not write any more data to the socket.
				if (!ec_linger && !(lnger.enabled() == true && lnger.timeout() == 0))
				{
					error_code ec_shutdown{};

					derive.socket().shutdown(asio::socket_base::shutdown_send, ec_shutdown);

					// if the reading is true , it means that the shutdown is called by manual.
					// if the reading is false, it means that the shutdown is called by the recv
					// error.
					// 
					// after call shutdown, the recv function should be returned with error, but
					// when use our http server and the client is pc broswer, even if we has called
					// shutdown, the recv function still won't be returned, so we must use a timeout
					// timer to ensure the closesocket can be called.
					// 
					// and some times, even if we has called shutdown, the http session' recv fucntion
					// will returned with no error.

					if (!ec_shutdown && derive.reading_)
					{
						derive._make_shutdown_timer(ec, std::move(this_ptr), std::move(chain));

						return;
					}
				}
			}

			derive._handle_shutdown(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename DeferEvent>
		inline void _handle_shutdown(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			ASIO2_LOG_DEBUG("shutdown_cp::_handle_shutdown: {} {}", ec.value(), ec.message());

			this->shutdown_timer_.reset();

			derive._do_close(ec, std::move(this_ptr), std::move(chain));
		}

	protected:
		template<typename DeferEvent>
		inline void _make_shutdown_timer(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.io_->running_in_this_thread());

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, ec, this_ptr = std::move(this_ptr), chain = std::move(chain)]() mutable
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				ASIO2_ASSERT(this->shutdown_timer_ == nullptr);

				if (this->shutdown_timer_)
				{
					this->shutdown_timer_->cancel();
				}

				this->shutdown_timer_ = std::make_shared<safe_timer>(derive.io_->context());

				derive._post_shutdown_timer(ec, std::move(this_ptr), std::move(chain),
					derive.get_disconnect_timeout(), this->shutdown_timer_);
			}));
		}

		template<typename DeferEvent, class Rep, class Period>
		inline void _post_shutdown_timer(
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain,
			std::chrono::duration<Rep, Period> duration, std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->shutdown_timer_.get())
				return;

			safe_timer* ptimer = timer_ptr.get();

			ptimer->timer.expires_after(duration);
			ptimer->timer.async_wait(
			[&derive, ec, this_ptr = std::move(this_ptr), chain = std::move(chain), timer_ptr = std::move(timer_ptr)]
			(const error_code& timer_ec) mutable
			{
				derive._handle_shutdown_timer(
					timer_ec, ec, std::move(this_ptr), std::move(chain), std::move(timer_ptr));
			});
		}

		template<typename DeferEvent>
		inline void _handle_shutdown_timer(
			const error_code& timer_ec,
			const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain,
			std::shared_ptr<safe_timer> timer_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT((!timer_ec) || timer_ec == asio::error::operation_aborted);

			// a new timer is maked, this is the prev timer, so return directly.
			if (timer_ptr.get() != this->shutdown_timer_.get())
				return;

			// member variable timer should't be empty
			if (!this->shutdown_timer_)
			{
				ASIO2_ASSERT(false);
				return;
			}

			// current timer is canceled by manual
			if (timer_ec == asio::error::operation_aborted || timer_ptr->canceled.test_and_set())
			{
				timer_ptr->canceled.clear();

				ASIO2_LOG_DEBUG("shutdown_cp::_handle_shutdown_timer: canceled");

				derive._handle_shutdown(ec, std::move(this_ptr), std::move(chain));
			}
			// timeout
			else
			{
				timer_ptr->canceled.clear();

				std::chrono::system_clock::duration silence = derive.get_silence_duration();

				// if recvd data in shutdown timeout period, lengthened the timer to remained times.
				if (silence < derive.get_disconnect_timeout())
				{
					derive._post_shutdown_timer(ec, std::move(this_ptr), std::move(chain),
						derive.get_disconnect_timeout() - silence, std::move(timer_ptr));
				}
				else
				{
					ASIO2_LOG_DEBUG("shutdown_cp::_handle_shutdown_timer: timeout");

					derive._handle_shutdown(ec, std::move(this_ptr), std::move(chain));
				}
			}
		}

		inline void _stop_shutdown_timer(std::shared_ptr<derived_t> this_ptr)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, this_ptr = std::move(this_ptr)]() mutable
			{
				if (this->shutdown_timer_)
				{
					this->shutdown_timer_->cancel();
				}
			}));
		}

	protected:
		/// beacuse the shutdown timer is used only when shutdown, so we use a pointer
		/// to reduce memory space occupied when running
		std::shared_ptr<safe_timer> shutdown_timer_;
	};
}

#endif // !__ASIO2_SHUTDOWN_COMPONENT_HPP__
