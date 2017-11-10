/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_CONNECTION_IMPL_HPP__
#define __ASIO2_TCP_CONNECTION_IMPL_HPP__

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

#include <asio2/base/connection_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

namespace asio2
{

	template<class _pool_t>
	class tcp_connection_impl : public connection_impl
	{
	public:

		typedef _pool_t pool_t;

		using connection_impl::send;

		/**
		 * @construct
		 */
		explicit tcp_connection_impl(
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<io_service> recv_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: connection_impl(
				send_ioservice_ptr,
				recv_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr
			)
			, m_send_buf_pool_ptr(send_buf_pool_ptr)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_connection_impl()
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

			// reset the state to the default
			m_stop_is_called = false;
			m_fire_close_is_called.clear(std::memory_order_release);

			try
			{
				boost::asio::ip::tcp::endpoint bind_endpoint(boost::asio::ip::address_v4::any(), 0);

				// socket contructor function with endpoint param will automic call open and bind.
				m_socket_ptr = std::make_shared<boost::asio::ip::tcp::socket>(*m_recv_ioservice_ptr, bind_endpoint);

				//m_socket_ptr->open(endpoint.protocol());
				//m_socket_ptr->bind(endpoint);

				boost::asio::ip::tcp::resolver resolver(*m_recv_ioservice_ptr);
				boost::asio::ip::tcp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::tcp::endpoint server_endpoint = *resolver.resolve(query);
				//boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
				//boost::asio::connect(socket, endpoint_iterator);

				if (async_connect)
				{
					m_socket_ptr->async_connect(server_endpoint,
						m_recv_strand_ptr->wrap(std::bind(&tcp_connection_impl::_handle_connect,
							std::dynamic_pointer_cast<tcp_connection_impl>(shared_from_this()),
							std::placeholders::_1
						)));

					return (is_start());
				}
				else
				{
					boost::system::error_code ec;
					m_socket_ptr->connect(server_endpoint, ec);

					_handle_connect(ec);

					// if error code is not 0,then connect failed,return false
					return (ec == 0 && is_start());
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			catch (std::exception &)
			{
			}

			return false;
		}

		/**
		 * @function : stop client,this function may be blocking 
		 */
		virtual void stop() override
		{
			bool is_start = this->is_start();

			m_stop_is_called = true;

			// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
			// can get notify to exit
			if (m_socket_ptr && m_socket_ptr->is_open())
			{
				// close the socket by post a event
				if (is_start)
				{
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					try
					{
						// use promise and future to wait for the async post finished.
						std::promise<void> promise_send;

						// first wait for all send pending complete
						m_send_strand_ptr->post([this, &promise_send]()
						{
							// do nothing ,just make sure the send pending is last executed
							auto_promise<void> ap(promise_send);
						});

						promise_send.get_future().wait();

						// when call shared_from_this ,may be the shared_ptr of "this" has disappeared already,so call shared_from_this will
						// cause exception,and we should't post event again,
						auto this_ptr = std::dynamic_pointer_cast<tcp_connection_impl>(shared_from_this());
						m_recv_strand_ptr->post([this_ptr]()
						{
							this_ptr->_close_socket();
						});
					}
					catch (std::exception &) {}
				}
			}
		}

		/**
		 * @function : whether the client is started
		 */
		virtual bool is_start()
		{
			return (!m_stop_is_called && m_socket_ptr && m_socket_ptr->is_open());
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr) override
		{
			// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
			try
			{
				if (is_start())
				{
					m_send_strand_ptr->post(std::bind(&tcp_connection_impl::_post_send,
						std::static_pointer_cast<tcp_connection_impl>(shared_from_this()),
						send_buf_ptr
					));
					return true;
				}
			}
			catch (std::exception &) {}
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
		inline std::shared_ptr<boost::asio::ip::tcp::socket> get_socket_ptr()
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->local_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->local_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->remote_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->remote_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::receive_buffer_size option(size);
					m_socket_ptr->set_option(option);
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::send_buffer_size option(size);
					m_socket_ptr->set_option(option);
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
			if (!m_socket_ptr || !m_socket_ptr->is_open())
				return false;

			boost::asio::socket_base::keep_alive option(onoff);
			m_socket_ptr->set_option(option);

#if defined(__unix__) || defined(__linux__)
			// For *n*x systems
			int native_fd = m_socket_ptr->native();

			int ret_keepidle = setsockopt(native_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&timeout, sizeof(unsigned int));
			int ret_keepintvl = setsockopt(native_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&interval, sizeof(unsigned int));
			int ret_keepinit = setsockopt(native_fd, SOL_TCP, TCP_KEEPCNT, (void*)&count, sizeof(unsigned int));

			if (ret_keepidle || ret_keepintvl || ret_keepinit)
			{
				return false;
			}

#elif defined(__OSX__)
			int native_fd = m_socket_ptr->native();

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

			if (SOCKET_ERROR == ::WSAIoctl(m_socket_ptr->native(), SIO_KEEPALIVE_VALS, (LPVOID)&keepalive_options, (DWORD)sizeof(keepalive_options),
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
			if (m_socket_ptr && m_socket_ptr->is_open())
			{
				boost::system::error_code ec;

				m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
				if (ec)
					set_last_error(ec.value());

				m_socket_ptr->close(ec);
				if (ec)
					set_last_error(ec.value());
			}
		}

		virtual void _handle_connect(const boost::system::error_code& ec)
		{
			set_last_error(ec.value());

			_fire_connect(ec.value());

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
				_post_recv(shared_from_this());
			}
		}

		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr)
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				m_socket_ptr->async_read_some(
					boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
					m_recv_strand_ptr->wrap(std::bind(&tcp_connection_impl::_handle_recv, std::static_pointer_cast<tcp_connection_impl>(this_ptr),
						std::placeholders::_1, // error_code
						std::placeholders::_2, // bytes_recvd
						this_ptr,
						recv_buf_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				if (bytes_recvd == 0)
				{
					// recvd data len is 0,may be heartbeat packet.
				}
				else if (bytes_recvd > 0)
				{
					recv_buf_ptr->resize(bytes_recvd);
				}

				_fire_recv(recv_buf_ptr);

				_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				// close this session
				_fire_close(ec.value());
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				boost::asio::write(*m_socket_ptr, boost::asio::buffer(send_buf_ptr->data(), send_buf_ptr->size()), ec);
				set_last_error(ec.value());
				_fire_send(send_buf_ptr, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;

					_fire_close(ec.value());
				}
			}
		}

		virtual void _fire_connect(int error)
		{
			std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_connect(error);
		}

		virtual void _fire_recv(std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_recv(recv_buf_ptr);
		}

		virtual void _fire_send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr, int error)
		{
			std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_send(send_buf_ptr, error);
		}

		virtual void _fire_close(int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_close(error);

				_close_socket();
			}
		}

	protected:

		std::shared_ptr<boost::asio::ip::tcp::socket> m_socket_ptr;

		/// send buffer pool
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};

}

#endif // !__ASIO2_TCP_CONNECTION_IMPL_HPP__
