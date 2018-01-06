/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_SESSION_IMPL_HPP__
#define __ASIO2_TCP_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/session_impl.hpp>

namespace asio2
{

	class tcp_session_impl : public session_impl
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _session_impl_t> friend class http_acceptor_impl;

	public:
		/**
		 * @construct
		 */
		explicit tcp_session_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr,
			std::shared_ptr<boost::asio::io_context>       io_context_ptr,
			std::shared_ptr<session_mgr>                   session_mgr_ptr
		)
			: session_impl(url_parser_ptr, listener_mgr_ptr, io_context_ptr, session_mgr_ptr)
			, m_socket(*io_context_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_session_impl()
		{
		}

		/**
		 * @function : start this session for prepare to recv msg
		 */
		virtual bool start() override
		{
			m_state = state::starting;

			// reset the variable to default status
			m_fire_close_is_called.clear(std::memory_order_release);

			m_state = state::started;

			auto self(shared_from_this());

			_fire_accept(self);

			// user has called stop in the listener function,so we can't start continue.
			if (m_state != state::started)
				return false;

			try
			{
				// set keeplive
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
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}

			m_state = state::running;

			if (!this->m_session_mgr_ptr->start(self))
			{
				this->stop();
				return false;
			}

			// start the timer of check silence timeout
			if (m_url_parser_ptr->get_silence_timeout() > 0)
			{
				m_timer.expires_from_now(boost::posix_time::seconds(m_url_parser_ptr->get_silence_timeout()));
				m_timer.async_wait(
					m_strand_ptr->wrap(std::bind(&tcp_session_impl::_handle_timer, this,
						std::placeholders::_1, // error_code
						self
					)));
			}

			// to avlid the user call stop in another thread,then it may be m_socket.async_read_some and m_socket.close be called at the same time
			this->m_strand_ptr->post([this, self]()
			{
				this->_post_recv(std::move(self), std::make_shared<buffer<uint8_t>>(
					m_url_parser_ptr->get_recv_buffer_size(), malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), 0));
			});

			return true;
		}

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() override
		{
			if (m_state >= state::starting)
			{
				auto prev_state = m_state;
				m_state = state::stopping;

				// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
				// can get notify to exit
				if (m_socket.is_open())
				{
					// close the socket by post a event
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					try
					{
						auto self(shared_from_this());
						m_strand_ptr->post([this, self, prev_state]()
						{
							auto this_ptr = std::const_pointer_cast<session_impl>(self);
							try
							{
								if (prev_state == state::running)
									_fire_close(this_ptr, get_last_error());

								// close the socket
								m_socket.shutdown(boost::asio::socket_base::shutdown_both);
								m_socket.close();

								// clost the timer
								m_timer.cancel();
							}
							catch (boost::system::system_error & e)
							{
								set_last_error(e.code().value());
							}

							m_state = state::stopped;

							// remove this session from the session map
							m_session_mgr_ptr->stop(this_ptr);
						});
					}
					catch (std::exception &) {}
				}
			}
		}

		/**
		 * @function : whether the session is started
		 */
		virtual bool is_started() override
		{
			return ((m_state >= state::started) && m_socket.is_open());
		}

		/**
		 * @function : check whether the session is stopped
		 */
		virtual bool is_stopped() override
		{
			return ((m_state == state::stopped) && !m_socket.is_open());
		}

		/**
		 * @function : send data
		 * note : cannot be executed at the same time in multithreading when "async == false"
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			// We must ensure that there is only one operation to send data at the same time,otherwise may be cause crash.
			if (is_started() && buf_ptr)
			{
				try
				{
					// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
					m_strand_ptr->post(std::bind(&tcp_session_impl::_post_send, this,
						shared_from_this(),
						std::move(buf_ptr)
					));
					return true;
				}
				catch (std::exception &) {}
			}
			else if (!m_socket.is_open())
			{
				set_last_error((int)errcode::socket_not_ready);
			}
			return false;
		}

	public:
		/**
		 * @function : get the socket refrence
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
		virtual void * _get_key() override
		{
			return static_cast<void *>(this);
		}

		virtual void _handle_timer(const boost::system::error_code & ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// silence duration seconds not exceed the silence timeout,post a timer event agagin to avoid this session shared_ptr
				// object disappear.
				if (get_silence_duration() < m_url_parser_ptr->get_silence_timeout() * 1000)
				{
					auto remain = (m_url_parser_ptr->get_silence_timeout() * 1000) - get_silence_duration();
					m_timer.expires_from_now(boost::posix_time::milliseconds(remain));
					m_timer.async_wait(
						m_strand_ptr->wrap(std::bind(&tcp_session_impl::_handle_timer, this,
							std::placeholders::_1,
							std::move(this_ptr)
						)));
				}
				else
				{
					// silence timeout has elasped,but has't data trans,don't post a timer event again,so this session shared_ptr will
					// disappear and the object will be destroyed automatically after this handler returns.
					this->stop();
				}
			}
			else
			{
				// occur error,may be cancel is called
				this->stop();
			}
		}

		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (this->is_started())
			{
				if (buf_ptr->remain() > 0)
				{
					const auto & buffer = boost::asio::buffer(buf_ptr->write_begin(), buf_ptr->remain());
					this->m_socket.async_read_some(buffer,
						this->m_strand_ptr->wrap(std::bind(&tcp_session_impl::_handle_recv, this,
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

		virtual void _handle_recv(const boost::system::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				this->reset_last_active_time();

				buf_ptr->write_bytes(bytes_recvd);

				auto use_count = buf_ptr.use_count();

				this->_fire_recv(this_ptr, buf_ptr);

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

				this->stop();
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		virtual void _post_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_started())
			{
				boost::system::error_code ec;
				boost::asio::write(m_socket, boost::asio::buffer((void *)buf_ptr->read_begin(), buf_ptr->size()), ec);
				set_last_error(ec.value());
				this->_fire_send(this_ptr, buf_ptr, ec.value());
				if (ec)
				{
					PRINT_EXCEPTION;
					this->stop();
				}
			}
			else
			{
				set_last_error((int)errcode::socket_not_ready);
				this->_fire_send(this_ptr, buf_ptr, get_last_error());
			}
		}

		virtual void _fire_accept(std::shared_ptr<session_impl> & this_ptr)
		{
			static_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_accept(this_ptr);
		}

		virtual void _fire_recv(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		{
			static_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_recv(this_ptr, buf_ptr);
		}

		virtual void _fire_send(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr, int error)
		{
			static_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_send(this_ptr, buf_ptr, error);
		}

		virtual void _fire_close(std::shared_ptr<session_impl> & this_ptr, int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				static_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_close(this_ptr, error);
			}
		}

	protected:
		/// tcp socket
		boost::asio::ip::tcp::socket m_socket;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;
	};

}

#endif // !__ASIO2_TCP_SESSION_IMPL_HPP__
