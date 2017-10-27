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

#include <asio2/util/pool.hpp>

#include <asio2/base/session_impl.hpp>
#include <asio2/base/session_mgr.hpp>

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
		 * @param    : io_service_evt - the io_service used to handle the socket event
		 *           : io_service_msg - the io_service used to handle the received msg
		 */
		explicit tcps_session_impl(
			io_service_ptr evt_ioservice_ptr,
			io_service_ptr msg_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<_pool_t> recv_buf_pool_ptr
		)
			: session_impl(evt_ioservice_ptr, msg_ioservice_ptr, listener_mgr_ptr, url_parser_ptr)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
		{
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

			// first call base class start function
			if (!session_impl::start())
				return false;

			if (is_start())
			{
				m_async_notify = (m_url_parser_ptr->get_param_value("notify_mode") == "async");

				// set keeplive
				set_keepalive_vals();

				// set send buffer size from url params
				_set_send_buffer_size_from_url();

				// set recv buffer size from url params
				_set_recv_buffer_size_from_url();

				_post_handshake();

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
						m_evt_strand_ptr->post([this_ptr]()
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
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
		{
			if (is_start())
			{
				try
				{
					// note : can't use m_msg_ioservice_ptr to post event,because can't operate socket in multi thread.
					// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
					m_evt_strand_ptr->post(std::bind(&tcps_session_impl::_post_send,
						std::static_pointer_cast<tcps_session_impl>(shared_from_this()),
						send_buf_ptr,
						len
					));
					return true;
				}
				catch (std::exception & e)
				{
					set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
					PRINT_EXCEPTION;
				}
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
				set_last_error(e.code().value(), e.what());
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
				set_last_error(e.code().value(), e.what());
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
				set_last_error(e.code().value(), e.what());
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
				set_last_error(e.code().value(), e.what());
			}
			return 0;
		}

		/**
		 * @function : get socket's recv buffer size
		 */
		virtual int get_recv_buffer_size() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					boost::asio::socket_base::receive_buffer_size option;
					m_socket_ptr->lowest_layer().get_option(option);
					return option.value();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
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
				set_last_error(e.code().value(), e.what());
			}
			return false;
		}

		/**
		 * @function : get socket's send buffer size
		 */
		virtual int get_send_buffer_size() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
				{
					boost::asio::socket_base::send_buffer_size option;
					m_socket_ptr->lowest_layer().get_option(option);
					return option.value();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
			}
			return 0;
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
				set_last_error(e.code().value(), e.what());
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

		tcps_session_impl & set_context(std::shared_ptr<boost::asio::ssl::context> context_ptr)
		{
			m_context_ptr = context_ptr;

			if (m_evt_ioservice_ptr && m_context_ptr)
			{
				m_socket_ptr = std::make_shared<ssl_socket>(*m_evt_ioservice_ptr, *m_context_ptr);

				m_timer_ptr = std::make_shared<boost::asio::deadline_timer>(*m_evt_ioservice_ptr);
			}

			return (*this);
		}

	protected:

		/**
		 * @function : reset the resource to default status
		 */
		virtual void _reset() override
		{
			session_impl::_reset();

			m_async_notify = false;

			m_stop_is_called = false;
			m_fire_close_is_called.clear(std::memory_order_release);

			m_context_ptr.reset();
		}

		/**
		 * @function : colse the socket
		 */
		virtual void _close_socket() override
		{
			if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
			{
				auto this_ptr = std::static_pointer_cast<tcps_session_impl>(shared_from_this());

				m_timer_ptr->expires_from_now(boost::posix_time::seconds(60));
				m_timer_ptr->async_wait(m_evt_strand_ptr->wrap([this_ptr](const boost::system::error_code& err)
				{
					boost::system::error_code ec;

					// when the lowest socket is closed,the ssl stream shutdown will returned.
					this_ptr->m_socket_ptr->lowest_layer().shutdown(boost::asio::socket_base::shutdown_both, ec);
					if (ec)
						set_last_error(ec.value(), ec.message());

					this_ptr->m_socket_ptr->lowest_layer().close(ec);
					if (ec)
						set_last_error(ec.value(), ec.message());
				}));

				// when server call ssl stream shutdown first,if the client socket is not closed forever,then here shutdowm will blocking forever.
				m_socket_ptr->async_shutdown(m_evt_strand_ptr->wrap([this_ptr](const boost::system::error_code & ec)
				{
					this_ptr->m_timer_ptr->cancel();

					if ((ec.category() == boost::asio::error::get_ssl_category())/* && (SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(ec.value()))*/)
					{
						//ssl_stream.lowest_layer().close();
						set_last_error(ec.value(), ec.message());
					}

					if (ec.category() == boost::asio::error::get_ssl_category() && ec.value() == boost::asio::ssl::error::stream_truncated)
					{
						// -> not a real error:
						//do_ssl_async_shutdown();
						set_last_error(ec.value(), ec.message());
					}

					if (ec)
						set_last_error(ec.value(), ec.message());
				}));
			}
		}

		virtual void * _get_key() override
		{
			return static_cast<void *>(&_key);
		}

		virtual void _post_handshake()
		{
			if (is_start())
			{
				m_socket_ptr->async_handshake(boost::asio::ssl::stream_base::server,
					m_evt_strand_ptr->wrap(std::bind(&tcps_session_impl::_handle_handshake, std::static_pointer_cast<tcps_session_impl>(shared_from_this()),
						std::placeholders::_1 // error_code
					)));
			}
		}

		virtual void _handle_handshake(const boost::system::error_code& ec)
		{
			if (!ec)
			{
				_post_recv();
			}
			else
			{
				set_last_error(ec.value(), ec.message());

				_fire_close(ec.value());
			}
		}

		virtual void _post_recv()
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<uint8_t> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				m_socket_ptr->async_read_some(
					boost::asio::buffer(recv_buf_ptr.get(), m_recv_buf_pool_ptr->get_requested_size()),
					m_evt_strand_ptr->wrap(std::bind(&tcps_session_impl::_handle_recv, std::static_pointer_cast<tcps_session_impl>(shared_from_this()),
						std::placeholders::_1, // error_code
						std::placeholders::_2, // bytes_recvd
						recv_buf_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr)
		{
			// every times recv data,we update the last active time.
			reset_last_active_time();

			if ((ec.category() == boost::asio::error::get_ssl_category())/* && (SSL_R_PROTOCOL_IS_SHUTDOWN == ERR_GET_REASON(ec.value()))*/)
			{
				//ssl_stream.lowest_layer().close();
				set_last_error(ec.value(), ec.message());
			}

			if (ec.category() == boost::asio::error::get_ssl_category() && ec.value() == boost::asio::ssl::error::stream_truncated)
			{
				// -> not a real error:
				//do_ssl_async_shutdown();
				set_last_error(ec.value(), ec.message());
			}

			if (!ec)
			{
				_fire_recv(recv_buf_ptr, bytes_recvd);

				_post_recv();
			}
			else
			{
				set_last_error(ec.value());

				_fire_close(ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				size_t bytes_sent = boost::asio::write(*m_socket_ptr, boost::asio::buffer(send_buf_ptr.get(), len), ec);
				set_last_error(ec.value());
				_fire_send(send_buf_ptr, bytes_sent, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;

					_fire_close(ec.value());
				}
			}
		}

		virtual void _fire_recv(std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
		{
			// when recv one msg,we don't handle it in this socket thread,instead we handle it in another thread by io_service.post function,
			// note : after we post the msg,if the post handler function _do_fire_recv is need long time to handle the msg,will cause the shared_ptr 
			// "recv_buf_ptr" not released,the recv_buf_ptr is allocated with default 1024 bytes buffer,when the remain recv_buf_ptr is too many,
			// will cause the memory use is too big.
			try
			{
				if (is_start() && std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->is_recv_listener_exist())
				{
					if (m_async_notify)
					{
						m_msg_strand_ptr->post(std::bind(&tcps_session_impl::_do_fire_recv,
							std::static_pointer_cast<tcps_session_impl>(shared_from_this()),
							recv_buf_ptr, bytes_recvd));
					}
					else
					{
						_do_fire_recv(recv_buf_ptr, bytes_recvd);
					}
				}
			}
			catch (std::exception &) {}
		}

		virtual void _do_fire_recv(std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
		{
			try
			{
				std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_recv(shared_from_this(), recv_buf_ptr, bytes_recvd);
			}
			catch (std::exception &) {}
		}

		virtual void _fire_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t bytes_sent, int error)
		{
			try
			{
				if (is_start() && std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->is_send_listener_exist())
				{
					if (m_async_notify)
					{
						m_msg_strand_ptr->post(std::bind(&tcps_session_impl::_do_fire_send,
							std::static_pointer_cast<tcps_session_impl>(shared_from_this()),
							send_buf_ptr, bytes_sent, error));
					}
					else
					{
						_do_fire_send(send_buf_ptr, bytes_sent, error);
					}
				}
			}
			catch (std::exception &) {}
		}

		virtual void _do_fire_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t bytes_sent, int error)
		{
			try
			{
				std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_send(shared_from_this(), send_buf_ptr, bytes_sent, error);
			}
			catch (std::exception &) {}
		}

		virtual void _fire_close(int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				_do_fire_close(error);
			}
		}

		virtual void _do_fire_close(int error)
		{
			try
			{
				std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_close(shared_from_this(), error);
			}
			catch (std::exception &) {}

			_close_socket();
		}

	protected:

		std::shared_ptr<ssl_socket> m_socket_ptr;

		/// ssl context 
		std::shared_ptr<boost::asio::ssl::context> m_context_ptr;

		/// Buffer pool used to store data received from the client. 
		std::shared_ptr<_pool_t> m_recv_buf_pool_ptr;

		/// used for session_mgr's session unordered_map key
		tcps_session_impl * _key = this;

		/// notify mode,async or sync
		bool m_async_notify = false;

		/// use to check whether the user call session stop in the listener
		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

		/// timer for session time out
		std::shared_ptr<boost::asio::deadline_timer> m_timer_ptr;

	};

}

#endif // !__ASIO2_TCPS_SESSION_IMPL_HPP__
