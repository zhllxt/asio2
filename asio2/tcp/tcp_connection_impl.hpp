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

#include <asio2/base/connection_impl.hpp>

namespace asio2
{

	class tcp_connection_impl : public connection_impl
	{
	public:
		using connection_impl::send;

		/**
		 * @construct
		 */
		explicit tcp_connection_impl(
			std::shared_ptr<url_parser>              url_parser_ptr,
			std::shared_ptr<listener_mgr>            listener_mgr_ptr,
			std::shared_ptr<boost::asio::io_context> send_io_context_ptr,
			std::shared_ptr<boost::asio::io_context> recv_io_context_ptr
		)
			: connection_impl(
				url_parser_ptr,
				listener_mgr_ptr,
				send_io_context_ptr,
				recv_io_context_ptr
			)
			, m_socket(*recv_io_context_ptr)
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
			m_state = state::starting;

			// reset the variable to default status
			m_fire_close_is_called.clear(std::memory_order_release);

			try
			{
				boost::asio::ip::tcp::endpoint local_endpoint(boost::asio::ip::address_v4::any(), 0);

				m_socket.open(local_endpoint.protocol());
				m_socket.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true)); // set port reuse
				m_socket.bind(local_endpoint);

				// Connect succeeded. set the keeplive values
				set_keepalive_vals(m_socket);

				// setsockopt SO_SNDBUF from url params
				if (m_url_parser_ptr->get_so_sndbuf_size() > 0)
				{
					boost::asio::socket_base::send_buffer_size option(m_url_parser_ptr->get_so_sndbuf_size());
					m_socket.set_option(option);
				}

				// setsockopt SO_RCVBUF from url params
				if (m_url_parser_ptr->get_so_rcvbuf_size() > 0)
				{
					boost::asio::socket_base::receive_buffer_size option(m_url_parser_ptr->get_so_rcvbuf_size());
					m_socket.set_option(option);
				}

				boost::asio::ip::tcp::resolver resolver(*m_recv_io_context_ptr);
				boost::asio::ip::tcp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::tcp::endpoint server_endpoint = *resolver.resolve(query);

				//boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
				//boost::asio::connect(socket, endpoint_iterator);

				if (async_connect)
				{
					m_socket.async_connect(server_endpoint,
						m_recv_strand_ptr->wrap(std::bind(&tcp_connection_impl::_handle_connect, this,
							std::placeholders::_1,
							shared_from_this()
						)));

					return (m_socket.is_open());
				}
				else
				{
					boost::system::error_code ec;
					m_socket.connect(server_endpoint, ec);

					_handle_connect(ec, shared_from_this());

					// if error code is not 0,then connect failed,return false
					return (ec == 0 && is_start());
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			return false;
		}

		/**
		 * @function : stop client,this function may be blocking 
		 */
		virtual void stop() override
		{
			if (m_state >= state::starting)
			{
				m_state = state::stopping;

				// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
				// can get notify to exit
				if (m_socket.is_open())
				{
					try
					{
						auto self(shared_from_this());

						// first wait for the send event finished
						auto promise_ptr = std::make_shared<std::promise<void>>();
						m_send_strand_ptr->post([this, self, promise_ptr]()
						{
							promise_ptr->set_value();
						});

						// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
						// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
						// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
						m_recv_strand_ptr->post([this, self, promise_ptr]()
						{
							// wait util the send event is finished compelted
							promise_ptr->get_future().wait();

							// close the socket
							try
							{
								if (m_state == state::running)
									_fire_close(get_last_error());

								m_socket.shutdown(boost::asio::socket_base::shutdown_both);
								m_socket.close();
							}
							catch (boost::system::system_error & e)
							{
								set_last_error(e.code().value());
							}

							m_state = state::stopped;
						});
					}
					catch (std::exception &) {}
				}
			}
		}

		/**
		 * @function : check whether the connection is started
		 */
		virtual bool is_start()
		{
			return ((m_state >= state::started) && m_socket.is_open());
		}

		/**
		 * @function : send data
		 * note : cannot be executed at the same time in multithreading when "async == false"
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			// We must ensure that there is only one operation to send data at the same time,otherwise may be cause crash.
			if (is_start() && buf_ptr)
			{
				try
				{
					// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
					m_send_strand_ptr->post(std::bind(&tcp_connection_impl::_post_send, this,
						shared_from_this(),
						std::move(buf_ptr)
					));
					return true;
				}
				catch (std::exception &) {}
			}
			return false;
		}

	public:
		/**
		 * @function : get the socket shared_ptr
		 */
		inline boost::asio::ip::tcp::socket & get_socket()
		{
			return m_socket;
		}

		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address() override
		{
			try
			{
				if (m_socket.is_open())
				{
					return m_socket.local_endpoint().address().to_string();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return std::string();
		}

		/**
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port() override
		{
			try
			{
				if (m_socket.is_open())
				{
					return m_socket.local_endpoint().port();
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
				if (m_socket.is_open())
				{
					return m_socket.remote_endpoint().address().to_string();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return std::string();
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port() override
		{
			try
			{
				if (m_socket.is_open())
				{
					return m_socket.remote_endpoint().port();
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return 0;
		}

	protected:
		virtual void _handle_connect(const boost::system::error_code & ec, std::shared_ptr<connection_impl> this_ptr)
		{
			set_last_error(ec.value());

			if (ec == 0)
				m_state = state::started;

			_fire_connect(ec.value());

			// Connect succeeded.
			if (ec == 0 && m_state == state::started)
			{
				// to avlid the user call stop in another thread,then it may be m_socket.async_read_some and m_socket.close be called at the same time
				this->m_recv_strand_ptr->post([this, this_ptr]()
				{
					// Connect succeeded. post recv request.
					this->_post_recv(std::move(this_ptr), std::make_shared<buffer<uint8_t>>(
						m_url_parser_ptr->get_recv_buffer_size(), malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), 0));
				});

				m_state = state::running;
			}
		}

		virtual void _post_recv(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_start())
			{
				if (buf_ptr->remain() > 0)
				{
					const auto & buffer = boost::asio::buffer(buf_ptr->write_begin(), buf_ptr->remain());
					this->m_socket.async_read_some(buffer,
						this->m_recv_strand_ptr->wrap(std::bind(&tcp_connection_impl::_handle_recv, this,
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							std::move(this_ptr),
							std::move(buf_ptr)
						)));
				}
				else
				{
					set_last_error((int)errcode::recv_buffer_size_too_small);
					PRINT_EXCEPTION;
					this->stop();
					assert(false);
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
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
					buf_ptr->write_bytes(bytes_recvd);
				}

				auto use_count = buf_ptr.use_count();

				_fire_recv(buf_ptr);

				if (use_count == buf_ptr.use_count())
				{
					buf_ptr->reset();
					this->_post_recv(std::move(this_ptr), std::move(buf_ptr));
				}
				else
				{
					this->_post_recv(std::move(this_ptr), std::make_shared<buffer<uint8_t>>(
						m_url_parser_ptr->get_recv_buffer_size(), malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), 0));
				}
			}
			else
			{
				set_last_error(ec.value());

				// close this session
				this->stop();
			}

			// No new asynchronous operations are started. This means that all shared_ptr
			// references to the connection object will disappear and the object will be
			// destroyed automatically after this handler returns. The connection class's
			// destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<connection_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				boost::asio::write(m_socket, boost::asio::buffer((void *)buf_ptr->read_begin(), buf_ptr->size()), ec);
				set_last_error(ec.value());
				this->_fire_send(buf_ptr, ec.value());
				if (ec)
				{
					PRINT_EXCEPTION;
					this->stop();
				}
			}
			else
			{
				set_last_error((int)errcode::socket_not_ready);
				this->_fire_send(buf_ptr, get_last_error());
			}
		}

		virtual void _fire_connect(int error)
		{
			static_cast<client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_connect(error);
		}

		virtual void _fire_recv(std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		{
			static_cast<client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_recv(buf_ptr);
		}

		virtual void _fire_send(std::shared_ptr<buffer<uint8_t>> & buf_ptr, int error)
		{
			static_cast<client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_send(buf_ptr, error);
		}

		virtual void _fire_close(int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				static_cast<client_listener_mgr *>(m_listener_mgr_ptr.get())->notify_close(error);
			}
		}

	protected:
		/// tcp socket
		boost::asio::ip::tcp::socket m_socket;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};

}

#endif // !__ASIO2_TCP_CONNECTION_IMPL_HPP__
