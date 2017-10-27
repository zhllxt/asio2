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

#include <asio2/util/pool.hpp>
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
				evt_recv_ioservice_ptr,
				msg_send_ioservice_ptr,
				msg_recv_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr
			)
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

			try
			{
				m_async_notify = (m_url_parser_ptr->get_param_value("notify_mode") == "async");

				boost::asio::ip::udp::endpoint bind_endpoint(boost::asio::ip::address_v4::any(), 0);
				// socket contructor function with endpoint param will automic call open and bind.
				m_socket_ptr = std::make_shared<boost::asio::ip::udp::socket>(
					*m_evt_recv_ioservice_ptr, bind_endpoint);

				//m_socket_ptr->open(endpoint.protocol());
				//m_socket_ptr->bind(endpoint);

				boost::asio::ip::udp::endpoint server_endpoint(
					boost::asio::ip::address::from_string(m_url_parser_ptr->get_ip()),
					static_cast<unsigned short>(std::atoi(m_url_parser_ptr->get_port().c_str())));

				if (async_connect)
				{
					m_socket_ptr->async_connect(server_endpoint,
						m_evt_recv_strand_ptr->wrap(std::bind(&udp_connection_impl::_handle_connect,
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
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
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
					m_evt_send_strand_ptr->post([this, &promise_send]()
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
					m_evt_recv_strand_ptr->post([this, &promise_recv]()
					{
						_close_socket();

						auto_promise<void> ap(promise_recv);
					});

					// wait util the socket is closed completed
					// must wait for the socket closed completed,otherwise when use m_evt_strand_ptr post a event to close the socket,
					// before socket closed ,the event thread join is returned already,and it will cause memory leaks

					// wait for the async task finish,when finished,the socket must be closed already.
					promise_recv.get_future().wait();
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
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
		{
			if (is_start())
			{
				// note : can't use m_io_service_msg to post event,because can't operate socket in multi thread.
				// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
				try
				{
					m_evt_send_strand_ptr->post(std::bind(&udp_connection_impl::_post_send,
						std::static_pointer_cast<udp_connection_impl>(shared_from_this()),
						send_buf_ptr, len));
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->local_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->remote_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->remote_endpoint();
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::receive_buffer_size option;
					m_socket_ptr->get_option(option);
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::receive_buffer_size option(size);
					m_socket_ptr->set_option(option);
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::send_buffer_size option;
					m_socket_ptr->get_option(option);
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
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::send_buffer_size option(size);
					m_socket_ptr->set_option(option);
					return true;
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
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
					set_last_error(ec.value(), ec.message());

				m_socket_ptr->close(ec);
				if (ec)
					set_last_error(ec.value(), ec.message());
			}
		}

		virtual void _handle_connect(const boost::system::error_code& ec)
		{
			set_last_error(ec.value(), ec.message());

			_fire_connect(ec);

			// Connect succeeded.
			if (!ec)
			{
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
				std::shared_ptr<uint8_t> recv_buf_ptr = m_recv_buf_pool_ptr->get();

				try
				{
					m_socket_ptr->async_receive(
						boost::asio::buffer(recv_buf_ptr.get(), m_recv_buf_pool_ptr->get_requested_size()),
						m_evt_recv_strand_ptr->wrap(std::bind(&udp_connection_impl::_handle_recv, std::static_pointer_cast<udp_connection_impl>(shared_from_this()),
							std::placeholders::_1,
							std::placeholders::_2,
							recv_buf_ptr
						)));
				}
				catch (std::exception & e)
				{
					set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
					PRINT_EXCEPTION;
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr)
		{
			set_last_error(ec.value());

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

		virtual void _post_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				size_t bytes_sent = m_socket_ptr->send(boost::asio::buffer(send_buf_ptr.get(), len), 0, ec);
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
			try
			{
				if (is_start() && std::static_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->is_recv_listener_exist())
				{
					if (m_async_notify)
					{
						// when recv one msg,we don't handle it in this socket thread,instead we handle it in another thread by io_service.post function.
						m_msg_recv_strand_ptr->post(std::bind(&udp_connection_impl::_do_fire_recv,
							std::static_pointer_cast<udp_connection_impl>(shared_from_this()),
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
						m_msg_send_strand_ptr->post(std::bind(&udp_connection_impl::_do_fire_send,
							std::static_pointer_cast<udp_connection_impl>(shared_from_this()),
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

		std::shared_ptr<boost::asio::ip::udp::socket> m_socket_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// notify mode,async or sync
		bool m_async_notify = false;

		/// use to check whether the user call session stop function
		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};
}

#endif // !__ASIO2_UDP_CONNECTION_IMPL_HPP__
