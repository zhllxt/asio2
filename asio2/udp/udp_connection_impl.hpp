/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_CONNECTION_IMPL_HPP__
#define __ASIO2_UDP_CONNECTION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>

#include <asio2/util/helper.hpp>

#include <asio2/base/client_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

namespace asio2
{

	template<class _pool_t>
	class udp_connection_impl : public connection_impl
	{
	public:

		using connection_impl::send;

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit udp_connection_impl(
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
		virtual ~udp_connection_impl()
		{
		}

		/**
		 * @function : start the client.you must call the stop function before application exit,otherwise will cause crash.
		 * @param    : port    - the listen ip port
		 *             address - the listen ip address
		 *             async_connect - async or sync connect to the server
		 * @return   : true  - start successed 
		 *             false - start failed
		 */
		virtual bool start(bool async_connect = false) override
		{
			// first call base class start function
			if (!connection_impl::start(async_connect))
				return false;

			// reset the state to the default
			m_stop_is_called = false;
			m_fire_close_is_called.clear(std::memory_order_release);

			try
			{
				boost::asio::ip::udp::endpoint bind_endpoint(boost::asio::ip::address_v4::any(), 0);
				// socket contructor function with endpoint param will automic call open and bind.
				m_socket_ptr = std::make_shared<boost::asio::ip::udp::socket>(
					*m_recv_ioservice_ptr, bind_endpoint);

				//m_socket_ptr->open(endpoint.protocol());
				//m_socket_ptr->bind(endpoint);

				// parse address and port
				boost::asio::ip::udp::resolver resolver(*m_recv_ioservice_ptr);
				boost::asio::ip::udp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::udp::endpoint server_endpoint = *resolver.resolve(query);

				if (async_connect)
				{
					m_socket_ptr->async_connect(server_endpoint,
						m_recv_strand_ptr->wrap(std::bind(&udp_connection_impl::_handle_connect,
							std::dynamic_pointer_cast<udp_connection_impl>(shared_from_this()),
							std::placeholders::_1
						)));

					return (m_socket_ptr && m_socket_ptr->is_open());
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
			catch (std::exception &)
			{
			}

			return false;
		}

		/**
		 * @function : stop the client
		 */
		virtual void stop() override
		{
			bool is_start = this->is_start();

			m_stop_is_called = true;

			// call listen socket's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
			// can get notify to exit
			if (m_socket_ptr && m_socket_ptr->is_open())
			{
				// close the socket by post a event
				if (is_start)
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

					// use promise and future to wait for the async post finished.
					std::promise<void> promise_recv;

					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_read... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					m_recv_strand_ptr->post([this, &promise_recv]()
					{
						_close_socket();

						auto_promise<void> ap(promise_recv);
					});

					// wait util the socket is closed completed
					// must wait for the socket closed completed,otherwise when use m_strand_ptr post a event to close the socket,
					// before socket closed ,the event thread join is returned already,and it will cause memory leaks

					// wait for the async task finish,when finished,the socket must be closed already.
					promise_recv.get_future().wait();
				}
			}

			m_stop_is_called = false;
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
			if (is_start() && send_buf_ptr)
			{
				// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
				try
				{
					m_send_strand_ptr->post(std::bind(&udp_connection_impl::_post_send,
						std::static_pointer_cast<udp_connection_impl>(shared_from_this()),
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
		inline std::shared_ptr<boost::asio::ip::udp::socket> get_socket_ptr()
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

			_fire_connect(ec);

			// Connect succeeded.
			if (!ec)
			{
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

				try
				{
					m_socket_ptr->async_receive(
						boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
						m_recv_strand_ptr->wrap(std::bind(&udp_connection_impl::_handle_recv, std::static_pointer_cast<udp_connection_impl>(this_ptr),
							std::placeholders::_1,
							std::placeholders::_2,
							this_ptr,
							recv_buf_ptr
						)));
				}
				catch (std::exception &)
				{
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			if (!ec)
			{
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

				// close this client
				_fire_close(ec.value());

				//// may be user pressed the CTRL + C to exit application.
				//if (ec == boost::asio::error::operation_aborted)
				//	return;

				//// if user call stop to stop client,the socket is closed,then _handle_recv will be called,and with error,so when appear error,we check 
				//// the socket status,if closed,don't call _post_recv again.
				//if (!m_socket_ptr || !m_socket_ptr->is_open())
				//	return;
			}
		}

		virtual void _post_send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				m_socket_ptr->send(boost::asio::buffer(send_buf_ptr->data(), send_buf_ptr->size()), 0, ec);
				set_last_error(ec.value());

				_fire_send(send_buf_ptr, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;

					_fire_close(ec.value());
				}
			}
		}

		virtual void _fire_connect(const boost::system::error_code& ec)
		{
			std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->notify_connect(ec.value());
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

		std::shared_ptr<boost::asio::ip::udp::socket> m_socket_ptr;

		/// send buffer pool
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// use to check whether the user call session stop function
		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};
}

#endif // !__ASIO2_UDP_CONNECTION_IMPL_HPP__
