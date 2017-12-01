/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_SESSION_IMPL_HPP__
#define __ASIO2_TCPS_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <vector>
#include <memory>
#include <thread>
#include <deque>
#include <mutex>

#if defined(__unix__) || defined(__linux__)
#elif defined(__OSX__)
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
#	include <mstcpip.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/error.hpp>

#include <asio2/base/session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcps_session_impl : public session_impl
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

		/**
		 * @construct
		 */
		explicit tcps_session_impl(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr,
			std::shared_ptr<boost::asio::ssl::context> context_ptr = nullptr
		)
			: session_impl(ioservice_ptr, listener_mgr_ptr, url_parser_ptr)
			, m_send_buf_pool_ptr(send_buf_pool_ptr)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
			, m_context_ptr(context_ptr)
		{
			if (m_ioservice_ptr && m_context_ptr)
			{
				m_socket_ptr = std::make_shared<ssl_socket>(*m_ioservice_ptr, *m_context_ptr);

				m_timer_ptr = std::make_shared<boost::asio::deadline_timer>(*m_ioservice_ptr);
			}
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_session_impl()
		{
		}

		/**
		 * @function : start this session for prepare to recv msg
		 */
		virtual bool start() override
		{
			// user has called stop in the listener function,so we can't start continue.
			if (m_stop_is_called)
				return false;

			// reset the variable to default status
			m_fire_close_is_called.clear(std::memory_order_release);

			// first call base class start function
			if (!session_impl::start())
				return false;

			if (is_start())
			{
				// set keeplive
				set_keepalive_vals();

				// set send buffer size from url params
				_set_send_buffer_size_from_url();

				// set recv buffer size from url params
				_set_recv_buffer_size_from_url();

				_post_handshake(shared_from_this());

				return true;
			}

			return false;
		}

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() override
		{
			bool is_start = this->is_start();

			m_stop_is_called = true;

			// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
			// can get notify to exit
			if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
			{
				// close the socket by post a event
				if (is_start)
				{
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					try
					{
						// when call shared_from_this ,may be the shared_ptr of "this" has disappeared already,so call shared_from_this will
						// cause exception,and we should't post event again,
						auto this_ptr = std::static_pointer_cast<tcps_session_impl>(shared_from_this());
						m_strand_ptr->post([this_ptr]()
						{
							this_ptr->_close_socket();
						});
					}
					catch (std::exception &) {}
				}
			}
		}

		/**
		 * @function : whether the session is started
		 */
		virtual bool is_start() override
		{
			return (!m_stop_is_called && m_socket_ptr && m_socket_ptr->lowest_layer().is_open());
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr) override
		{
			if (is_start() && send_buf_ptr)
			{
				try
				{
					auto this_ptr = shared_from_this();
					// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
					m_strand_ptr->post(std::bind(&tcps_session_impl::_post_send,
						std::static_pointer_cast<tcps_session_impl>(this_ptr),
						this_ptr,
						send_buf_ptr
					));
					return true;
				}
				catch (std::exception &)
				{
				}
			}
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len) override
		{
			if (is_start())
			{
				auto buf_ptr = this->m_send_buf_pool_ptr->get(get_power_number(len));
				std::memcpy((void *)buf_ptr->data(), (const void *)buf, len);
				buf_ptr->resize(len);
				return this->send(buf_ptr);
			}
			return false;
		}

	public:

		/**
		 * @function : get the socket shared_ptr
		 */
		inline std::shared_ptr<ssl_socket> get_socket_ptr()
		{
			return m_socket_ptr;
		}

		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					auto endpoint = m_socket_ptr->lowest_layer().local_endpoint();
					return endpoint.address().to_string();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return "";
		}

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					auto endpoint = m_socket_ptr->lowest_layer().local_endpoint();
					return endpoint.port();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return 0;
		}

		/**
		 * @function : get the remote address
		 */
		virtual std::string get_remote_address() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					auto endpoint = m_socket_ptr->lowest_layer().remote_endpoint();
					return endpoint.address().to_string();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return "";
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					auto endpoint = m_socket_ptr->lowest_layer().remote_endpoint();
					return endpoint.port();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return 0;
		}

		/**
		 * @function : set socket's recv buffer size.
		 *             when packet lost rate is high,you can set the recv buffer size to a big value to avoid it.
		 */
		virtual bool set_recv_buffer_size(int size) override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					boost::asio::socket_base::receive_buffer_size option(size);
					m_socket_ptr->lowest_layer().set_option(option);
					return true;
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return false;
		}

		/**
		 * @function : set socket's send buffer size
		 */
		virtual bool set_send_buffer_size(int size) override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					boost::asio::socket_base::send_buffer_size option(size);
					m_socket_ptr->lowest_layer().set_option(option);
					return true;
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return false;
		}

		/**
		 * @function : set tcp socket keep alive options
		 * @param    : onoff    - turn on or turn off
		 * @param    : timeout  - check time out 
		 * @param    : interval - check interval 
		 * @param    : count    - check times
		 */
		bool set_keepalive_vals(
			bool onoff = true,
			unsigned int timeout  = 30 * 1000,
			unsigned int interval = 10 * 1000,
			unsigned int count = 3
		)
		{
			if (!m_socket_ptr || !m_socket_ptr->lowest_layer().is_open())
				return false;

			boost::asio::socket_base::keep_alive option(onoff);
			m_socket_ptr->lowest_layer().set_option(option);

#if defined(__unix__) || defined(__linux__)
			// For *n*x systems
			int native_fd = m_socket_ptr->lowest_layer().native();

			int ret_keepidle = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&timeout, sizeof(unsigned int));
			int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
			int ret_keepinit = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT, (void*)&count, sizeof(unsigned int));

			if (ret_keepidle || ret_keepintvl || ret_keepinit)
			{
				return false;
			}

#elif defined(__OSX__)
			int native_fd = m_socket_ptr->lowest_layer().native();

			// Set the timeout before the first keep alive message
			int ret_tcpkeepalive = setsockopt(native_fd, IPPROTO_TCP, TCP_KEEPALIVE, (void*)&timeout, sizeof(unsigned int));
			int ret_tcpkeepintvl = setsockopt(native_fd, IPPROTO_TCP, TCP_CONNECTIONTIMEOUT, (void*)&interval, sizeof(unsigned int));

			if (ret_tcpkeepalive || ret_tcpkeepintvl)
			{
				return false;
			}

#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
			// Partially supported on windows
			tcp_keepalive keepalive_options;
			keepalive_options.onoff = onoff;
			keepalive_options.keepalivetime = timeout;
			keepalive_options.keepaliveinterval = interval;

			DWORD bytes_returned = 0;

			if (SOCKET_ERROR == ::WSAIoctl(m_socket_ptr->lowest_layer().native(), SIO_KEEPALIVE_VALS, (LPVOID)&keepalive_options, (DWORD)sizeof(keepalive_options),
				nullptr, 0, (LPDWORD)&bytes_returned, nullptr, nullptr))
			{
				if (::WSAGetLastError() != WSAEWOULDBLOCK)
					return false;
			}
#endif
			return true;
		}

	protected:
		/**
		 * @function : colse the socket
		 */
		virtual void _close_socket() override
		{
			session_impl::_close_socket();

			if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
			{
				auto this_ptr = std::static_pointer_cast<tcps_session_impl>(shared_from_this());

				m_timer_ptr->expires_from_now(boost::posix_time::seconds(60));
				m_timer_ptr->async_wait(m_strand_ptr->wrap([this_ptr](const boost::system::error_code& err)
				{
					boost::system::error_code ec;

					// when the lowest socket is closed,the ssl stream shutdown will returned.
					this_ptr->m_socket_ptr->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both, ec);
					if (ec)
						set_last_error(ec.value());

					this_ptr->m_socket_ptr->lowest_layer().close(ec);
					if (ec)
						set_last_error(ec.value());
				}));

				// when server call ssl stream shutdown first,if the client socket is not closed forever,then here shutdowm will blocking forever.
				m_socket_ptr->async_shutdown(m_strand_ptr->wrap([this_ptr](const boost::system::error_code & ec)
				{
					this_ptr->m_timer_ptr->cancel();

					//if ((ec.category() == boost::asio::error::get_ssl_category())/* && (SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(ec.value()))*/)
					//{
					//	//ssl_stream.lowest_layer().close();
					//	set_last_error(ec.value());
					//}

					//if (ec.category() == boost::asio::error::get_ssl_category() && ec.value() == boost::asio::ssl::error::stream_truncated)
					//{
					//	// -> not a real error:
					//	//do_ssl_async_shutdown();
					//	set_last_error(ec.value());
					//}

					if (ec)
						set_last_error(ec.value());
				}));
			}
			m_stop_is_called = false;
		}

		virtual void * _get_key() override
		{
			return static_cast<void *>(&_key);
		}

		virtual void _post_handshake(std::shared_ptr<session_impl> this_ptr)
		{
			if (is_start())
			{
				m_socket_ptr->async_handshake(boost::asio::ssl::stream_base::server,
					m_strand_ptr->wrap(std::bind(&tcps_session_impl::_handle_handshake, std::static_pointer_cast<tcps_session_impl>(this_ptr),
						std::placeholders::_1, // error_code
						this_ptr
					)));
			}
		}

		virtual void _handle_handshake(const boost::system::error_code& ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				_fire_close(this_ptr, ec.value());
			}
		}

		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr)
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				m_socket_ptr->async_read_some(
					boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
					m_strand_ptr->wrap(std::bind(&tcps_session_impl::_handle_recv, std::static_pointer_cast<tcps_session_impl>(this_ptr),
						std::placeholders::_1, // error_code
						std::placeholders::_2, // bytes_recvd
						this_ptr,
						recv_buf_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			//if ((ec.category() == boost::asio::error::get_ssl_category())/* && (SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(ec.value()))*/)
			//{
			//	//ssl_stream.lowest_layer().close();
			//	set_last_error(ec.value());
			//}

			//if (ec.category() == boost::asio::error::get_ssl_category() && ec.value() == boost::asio::ssl::error::stream_truncated)
			//{
			//	// -> not a real error:
			//	//do_ssl_async_shutdown();
			//	set_last_error(ec.value());
			//}

			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				recv_buf_ptr->resize(bytes_recvd);

				_fire_recv(this_ptr, recv_buf_ptr);

				_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				_fire_close(this_ptr, ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> send_buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				boost::asio::write(*m_socket_ptr, boost::asio::buffer(send_buf_ptr->data(),send_buf_ptr->size()), ec);
				set_last_error(ec.value());
				_fire_send(this_ptr, send_buf_ptr, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;

					_fire_close(this_ptr, ec.value());
				}
			}
		}

		virtual void _fire_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_recv(this_ptr, recv_buf_ptr);
		}

		virtual void _fire_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> send_buf_ptr, int error)
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_send(this_ptr, send_buf_ptr, error);
		}

		virtual void _fire_close(std::shared_ptr<session_impl> this_ptr, int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_close(this_ptr, error);

				_close_socket();
			}
		}

	protected:

		std::shared_ptr<ssl_socket> m_socket_ptr;

		/// ssl context 
		std::shared_ptr<boost::asio::ssl::context> m_context_ptr;

		/// send buffer pool
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// Buffer pool used to store data received from the client. 
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// used for session_mgr's session unordered_map key
		tcps_session_impl * _key = this;

		/// use to check whether the user call session stop in the listener
		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

		/// timer for session time out
		std::shared_ptr<boost::asio::deadline_timer> m_timer_ptr;

	};

}

#endif // !__ASIO2_TCPS_SESSION_IMPL_HPP__
