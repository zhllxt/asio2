/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_CONNECTION_IMPL_HPP__
#define __ASIO2_TCPS_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <thread>
#include <atomic>
#include <deque>

#if defined(__unix__) || defined(__linux__)
#elif defined(__OSX__)
#elif defined(WIN32) || defined(_WIN32) || defined(_WIN64) || defined(_WINDOWS_)
#	include <mstcpip.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <asio2/util/pool.hpp>

#include <asio2/base/connection_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcps_connection_impl : public connection_impl
	{
	public:

		using connection_impl::send;

		typedef _pool_t pool_t;
		typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

		/**
		 * @construct
		 */
		explicit tcps_connection_impl(
			io_service_ptr evt_send_ioservice_ptr,
			io_service_ptr evt_recv_ioservice_ptr,
			io_service_ptr msg_send_ioservice_ptr,
			io_service_ptr msg_recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: connection_impl(
				evt_send_ioservice_ptr,
				evt_send_ioservice_ptr, // ssl socket is half duplex,so we must ensure the send and recv on socket is single threaded sequential execution
				msg_send_ioservice_ptr,
				msg_send_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr
			)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_connection_impl()
		{
		}

		/**
		 * @function : start the client
		 * @param    : async_connect - asynchronous connect to the server or sync
		 * @return   : true  - start successed , false - start failed
		 */
		virtual bool start(bool async_connect = true) override
		{
			// first call base class start function
			if (!connection_impl::start(async_connect))
				return false;

			try
			{
				m_async_notify = (m_url_parser_ptr->get_param_value("notify_mode") == "async");

				m_timer_ptr = std::make_shared<boost::asio::deadline_timer>(*m_evt_recv_ioservice_ptr);

				m_socket_ptr = std::make_shared<ssl_socket>(*m_evt_recv_ioservice_ptr, *m_context_ptr);
				m_socket_ptr->set_verify_mode(boost::asio::ssl::verify_peer);
				m_socket_ptr->set_verify_callback(
					std::bind(&tcps_connection_impl::_verify_certificate, 
						// can't use shared_from_this,otherwise this function will hold the shared_from_this forever,
						// and cause the shared_ptr of "this" never disappeared.
						this, // std::dynamic_pointer_cast<tcps_connection_impl>(shared_from_this()),
						std::placeholders::_1, // preverified
						std::placeholders::_2  // boost::asio::ssl::verify_context
					));

				boost::asio::ip::tcp::resolver resolver(*m_evt_recv_ioservice_ptr);
				boost::asio::ip::tcp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

				if (async_connect)
				{
					boost::asio::async_connect(m_socket_ptr->lowest_layer(), iterator,
						m_evt_recv_strand_ptr->wrap(std::bind(&tcps_connection_impl::_handle_connect,
							std::dynamic_pointer_cast<tcps_connection_impl>(shared_from_this()),
							std::placeholders::_1 // error_code
						)));

					return (is_start());
				}
				else
				{
					boost::system::error_code ec;
					boost::asio::connect(m_socket_ptr->lowest_layer(), iterator, ec);

					_handle_connect(ec);

					// if error code is not 0,then connect failed,return false
					return (ec == 0 && is_start());
				}
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}

			return false;
		}

		/**
		 * @function : stop client,this function may be blocking 
		 */
		virtual void stop() override
		{
			// call listen socket's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
			// can get notify to exit
			if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
			{
				// close the socket by post a event
				if (is_start())
				{
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_read... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					auto this_ptr = std::dynamic_pointer_cast<tcps_connection_impl>(shared_from_this());
					m_evt_recv_strand_ptr->post([this_ptr]()
					{
						this_ptr->_close_socket();
					});
				}
			}
		}

		/**
		 * @function : whether the client is started
		 */
		virtual bool is_start()
		{
			return (
				//!m_stop_is_called &&
				(m_socket_ptr && m_socket_ptr->lowest_layer().is_open()) 
				);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
		{
			// note : can't use m_io_service_msg to post event,because can't operate socket in multi thread.
			// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
			try
			{
				if (is_start())
				{
					m_evt_send_strand_ptr->post(std::bind(&tcps_connection_impl::_post_send,
						std::static_pointer_cast<tcps_connection_impl>(shared_from_this()),
						send_buf_ptr, len));
					return true;
				}
			}
			catch (std::exception &) {}
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

		tcps_connection_impl & set_context(std::shared_ptr<boost::asio::ssl::context> context_ptr)
		{
			m_context_ptr = context_ptr;

			return (*this);
		}

	protected:

		/**
		 * @function : colse the socket
		 */
		virtual void _close_socket() override
		{
			if (m_socket_ptr && m_socket_ptr->lowest_layer().is_open())
			{
				auto this_ptr = std::dynamic_pointer_cast<tcps_connection_impl>(shared_from_this());

				m_timer_ptr->expires_from_now(boost::posix_time::seconds(60));
				m_timer_ptr->async_wait(m_evt_recv_strand_ptr->wrap([this_ptr](const boost::system::error_code& err)
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
				m_socket_ptr->async_shutdown(m_evt_recv_strand_ptr->wrap([this_ptr](const boost::system::error_code & ec)
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

		virtual bool _verify_certificate(bool preverified, boost::asio::ssl::verify_context & ctx)
		{
			// The verify callback can be used to check whether the certificate that is
			// being presented is valid for the peer. For example, RFC 2818 describes
			// the steps involved in doing this for HTTPS. Consult the OpenSSL
			// documentation for more details. Note that the callback is called once
			// for each certificate in the certificate chain, starting from the root
			// certificate authority.

			// In this example we will simply print the certificate's subject name.
			char subject_name[256];
			X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
			X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

			return preverified;
		}

		virtual void _handle_connect(const boost::system::error_code& ec)
		{
			set_last_error(ec.value(), ec.message());

			if (!ec)
			{
				_post_handshake();
			}
		}

		virtual void _post_handshake()
		{
			if (is_start())
			{
				m_socket_ptr->async_handshake(boost::asio::ssl::stream_base::client,
					m_evt_recv_strand_ptr->wrap(std::bind(&tcps_connection_impl::_handle_handshake,
						std::dynamic_pointer_cast<tcps_connection_impl>(shared_from_this()),
						std::placeholders::_1 // error_code
					)));
			}
		}

		virtual void _handle_handshake(const boost::system::error_code & ec)
		{
			set_last_error(ec.value(), ec.message());

			_fire_connect(ec);

			// Connect succeeded.
			if (!ec)
			{
				// Connect succeeded. set the keeplive values
				set_keepalive_vals();

				// set send buffer size from url params
				_set_send_buffer_size_from_url();

				// set recv buffer size from url params
				_set_recv_buffer_size_from_url();

				// Connect succeeded. post recv request.
				_post_recv();
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
					m_evt_recv_strand_ptr->wrap(std::bind(&tcps_connection_impl::_handle_recv, std::static_pointer_cast<tcps_connection_impl>(shared_from_this()),
						std::placeholders::_1,
						std::placeholders::_2,
						recv_buf_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr)
		{
			set_last_error(ec.value());

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
				if (bytes_recvd == 0)
				{
					// recvd data len is 0,may be heartbeat packet.
				}
				else if (bytes_recvd > 0)
				{
					// recvd data 
				}

				_fire_recv(recv_buf_ptr, bytes_recvd);

				_post_recv();
			}
			else
			{
				// close this session
				_fire_close(ec.value());
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
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

		virtual void _fire_connect(const boost::system::error_code& ec)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_connect(ec.value());
			}
			catch (std::exception &) {}
		}

		virtual void _fire_recv(std::shared_ptr<uint8_t> recv_buf_ptr, std::size_t bytes_recvd)
		{
			// when recv one msg,we don't handle it in this socket thread,instead we handle it in another thread by io_service.post function,
			// note : after we post the msg,if the post handler function _fire_recv is need long time to handle the msg,will cause the shared_ptr 
			// "recv_buf_ptr" not released,the recv_buf_ptr is allocated with default 1024 bytes buffer,when the remain recv_buf_ptr is too many,
			// will cause the memory use is too big.
			try
			{
				if (is_start() && std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->is_recv_listener_exist())
				{
					if (m_async_notify)
					{
						m_msg_recv_strand_ptr->post(std::bind(&tcps_connection_impl::_do_fire_recv,
							std::static_pointer_cast<tcps_connection_impl>(shared_from_this()),
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
				std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_recv(recv_buf_ptr, bytes_recvd);
			}
			catch (std::exception &) {}
		}

		virtual void _fire_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t bytes_sent, int error)
		{
			try
			{
				if (is_start() && std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->is_send_listener_exist())
				{
					if (m_async_notify)
					{
						m_msg_send_strand_ptr->post(std::bind(&tcps_connection_impl::_do_fire_send,
							std::static_pointer_cast<tcps_connection_impl>(shared_from_this()),
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
				std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_send(send_buf_ptr, bytes_sent, error);
			}
			catch (std::exception &) {}
		}

		virtual void _fire_close(int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
				_do_fire_close(error);
		}

		virtual void _do_fire_close(int error)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_close(error);
			}
			catch (std::exception &) {}

			_close_socket();
		}

	protected:

		std::shared_ptr<ssl_socket> m_socket_ptr;

		/// ssl context 
		std::shared_ptr<boost::asio::ssl::context> m_context_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// notify mode,async or sync
		bool m_async_notify = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

		/// timer for session time out
		std::shared_ptr<boost::asio::deadline_timer> m_timer_ptr;

	};

}

#endif // !__ASIO2_TCPS_CONNECTION_IMPL_HPP__
