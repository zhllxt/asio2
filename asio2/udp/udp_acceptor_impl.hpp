/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_UDP_ACCEPTOR_IMPL_HPP__
#define __ASIO2_UDP_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/acceptor_impl.hpp>

#include <asio2/udp/udp_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class udp_acceptor_impl : public acceptor_impl
	{
	public:
		typedef _session_impl_t session_impl_t;
		typedef typename _session_impl_t::_hasher hasher;
		typedef typename _session_impl_t::_equaler equaler;

		/**
		 * @construct
		 */
		udp_acceptor_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr,
			std::shared_ptr<io_context_pool>               io_context_pool_ptr
		)
			: acceptor_impl(url_parser_ptr, listener_mgr_ptr, io_context_pool_ptr)
			, m_acceptor(*m_io_context_ptr)
			, m_send_ioservice_ptr(io_context_pool_ptr->get_io_context_ptr())
		{
			m_send_strand_ptr = std::make_shared<asio::io_context::strand>(*m_send_ioservice_ptr);
			m_session_mgr_ptr = std::make_shared<session_mgr_t<asio::ip::udp::endpoint *, hasher, equaler>>();
		}

		/**
		 * @destruct
		 */
		virtual ~udp_acceptor_impl()
		{
		}

		/**
		 * @function : start acceptor
		 */
		virtual bool start() override
		{
			m_state = state::starting;

			try
			{
				// parse address and port
				asio::ip::udp::resolver resolver(*m_io_context_ptr);
				asio::ip::udp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				asio::ip::udp::endpoint endpoint = *resolver.resolve(query);

				m_acceptor.open(endpoint.protocol());

				// setsockopt SO_SNDBUF from url params
				if (m_url_parser_ptr->get_so_sndbuf_size() > 0)
				{
					asio::socket_base::send_buffer_size option(m_url_parser_ptr->get_so_sndbuf_size());
					m_acceptor.set_option(option);
				}

				// setsockopt SO_RCVBUF from url params
				if (m_url_parser_ptr->get_so_rcvbuf_size() > 0)
				{
					asio::socket_base::receive_buffer_size option(m_url_parser_ptr->get_so_rcvbuf_size());
					m_acceptor.set_option(option);
				}

				// when you close socket in linux system,and start socket immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct already before,why does this happen? the reasion is 
				// the socket option "TIME_WAIT",although you close the socket,but the system not release the socket,util 2~4 
				// seconds later,so we can use the SO_REUSEADDR option to avoid this problem,like below
				m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true)); // set port reuse
				m_acceptor.bind(endpoint);

				m_state = state::started;

				_fire_listen();

				// user has called stop in the listener function,so we can't start continue.
				if (m_state != state::started)
					return false;

				auto recv_buf = std::make_shared<buffer<uint8_t>>(m_url_parser_ptr->get_recv_buffer_size(), malloc_recv_buffer(m_url_parser_ptr->get_recv_buffer_size()), 0);

				_post_recv(shared_from_this(), std::move(recv_buf));

				m_state = state::running;
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());

				asio::error_code ec;
				m_acceptor.close(ec);
			}

			return (m_acceptor.is_open());
		}

		/**
		 * @function : stop acceptor
		 */
		virtual void stop() override
		{
			if (m_state >= state::starting)
			{
				auto prev_state = m_state;
				m_state = state::stopping;

				if (m_acceptor.is_open())
				{
					// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
					// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
					// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
					m_strand_ptr->post([this, prev_state]()
					{
						try
						{
							auto promise_ptr = std::make_shared<std::promise<void>>();

							// then stop all the sessions, the session::stop must be no blocking,otherwise it may be cause loop lock.
							m_session_mgr_ptr->destroy([this, promise_ptr]()
							{
								promise_ptr->set_value();
							});

							// wait util all session has closed already
							promise_ptr->get_future().wait();

							if (prev_state == state::running)
								_fire_shutdown(get_last_error());

							// close the socket
							m_acceptor.shutdown(asio::socket_base::shutdown_both);
							m_acceptor.close();

							m_state = state::stopped;
						}
						catch (asio::system_error & e)
						{
							set_last_error(e.code().value());
						}
					});
				}
			}
		}

		/**
		 * @function : test whether the acceptor is opened
		 */
		virtual bool is_started() override
		{
			return ((m_state >= state::started) && m_acceptor.is_open());
		}

		/**
		 * @function : check whether the acceptor is stopped
		 */
		virtual bool is_stopped() override
		{
			return ((m_state == state::stopped) && !m_acceptor.is_open());
		}

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() override
		{
			try
			{
				if (m_acceptor.is_open())
				{
					return m_acceptor.local_endpoint().address().to_string();
				}
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return std::string();
		}

		/**
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() override
		{
			try
			{
				if (m_acceptor.is_open())
				{
					return m_acceptor.local_endpoint().port();
				}
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return 0;
		}

		/**
		 * @function : get the acceptor shared_ptr
		 */
		inline asio::ip::udp::socket & get_acceptor()
		{
			return m_acceptor;
		}

	protected:
		virtual void _post_recv(std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_started())
			{
				if (buf_ptr->remain() > 0)
				{
					const auto & buffer = asio::buffer(buf_ptr->write_begin(), buf_ptr->remain());
					m_acceptor.async_receive_from(buffer,
						m_sender_endpoint,
						m_strand_ptr->wrap(std::bind(&udp_acceptor_impl::_handle_recv, this,
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							std::move(this_ptr),
							std::move(buf_ptr)
						)));
				}
				else
				{
					set_last_error((int)errcode::recv_buffer_size_too_small);
					ASIO2_DUMP_EXCEPTION_LOG_IMPL;
					this->stop();
					assert(false);
				}
			}
			else
			{
				set_last_error(asio::error::operation_aborted);
			}
		}

		virtual void _handle_recv(const asio::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_started())
			{
				auto use_count = buf_ptr.use_count();
				if (!ec)
				{
					buf_ptr->write_bytes(bytes_recvd);

					// first we find whether the session is in the session_mgr pool already,if not ,we new a session and put it into the session_mgr pool
					auto & session_ptr = m_session_mgr_ptr->find_session(static_cast<void *>(&m_sender_endpoint));

					if (!session_ptr)
					{
						std::shared_ptr<session_impl> new_session_ptr = _handle_accept();
						if (new_session_ptr)
						{
							static_cast<_session_impl_t *>(new_session_ptr.get())->_post_recv(new_session_ptr, buf_ptr);
						}
					}
					else
					{
						static_cast<_session_impl_t *>(session_ptr.get())->_post_recv(session_ptr, buf_ptr);
					}
				}
				else
				{
					set_last_error(ec.value());

					// may be user exit application.
					// if user call stop to stop server,the socket is closed,then _handle_recv will be called,and with error,so when appear error,we check 
					// the socket status,if closed,don't _post_recv again.
					if (ec == asio::error::operation_aborted)
						return;
				}

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
				set_last_error(asio::error::operation_aborted);
			}
		}

		virtual std::shared_ptr<session_impl> _make_session()
		{
			try
			{
				return std::make_shared<_session_impl_t>(
					this->m_url_parser_ptr,
					this->m_listener_mgr_ptr,
					this->m_io_context_ptr,
					this->m_strand_ptr,
					this->m_acceptor,
					this->m_send_ioservice_ptr,
					this->m_send_strand_ptr,
					this->m_sender_endpoint,
					this->m_session_mgr_ptr
					);
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());

				ASIO2_DUMP_EXCEPTION_LOG_IMPL;
			}

			return nullptr;
		}

		virtual std::shared_ptr<session_impl> _handle_accept()
		{
			auto session_ptr = _make_session();

			if (session_ptr)
			{
				session_ptr->start();
			}

			return session_ptr;
		}

		virtual void _fire_listen()
		{
			dynamic_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_listen();
		}

		virtual void _fire_shutdown(int error)
		{
			dynamic_cast<server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_shutdown(error);
		}

	protected:
		/// acceptor to accept client connection
		asio::ip::udp::socket   m_acceptor;

		/// endpoint for udp 
		asio::ip::udp::endpoint m_sender_endpoint;

		/// send io_context
		std::shared_ptr<asio::io_context>           m_send_ioservice_ptr;

		/// asio's strand to ensure asio.socket multi thread safe
		std::shared_ptr<asio::io_context::strand>   m_send_strand_ptr;

	};

}

#endif // !__ASIO2_UDP_ACCEPTOR_IMPL_HPP__
