/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_TCP_ACCEPTOR_IMPL_HPP__
#define __ASIO2_TCP_ACCEPTOR_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <chrono>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/base/acceptor_impl.hpp>
#include <asio2/base/session_mgr.hpp>

#include <asio2/tcp/tcp_session_impl.hpp>
#include <asio2/tcp/tcp_auto_session_impl.hpp>
#include <asio2/tcp/tcp_pack_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class tcp_acceptor_impl : public acceptor_impl
	{
	public:

		typedef _session_impl_t session_impl_t;
		typedef typename _session_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		tcp_acceptor_impl(
			io_service_pool_ptr ioservice_pool_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: acceptor_impl(ioservice_pool_ptr->get_io_service_ptr(), listener_mgr_ptr, url_parser_ptr)
			, m_ioservice_pool_ptr(ioservice_pool_ptr)
			, m_send_buf_pool_ptr(send_buf_pool_ptr)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_acceptor_impl()
		{
		}

		/**
		 * @function : start acceptor
		 */
		virtual bool start() override
		{
			if (!acceptor_impl::start())
				return false;

			try
			{
				// init session manager 
				m_session_mgr_ptr = std::make_shared<session_mgr_t<_session_impl_t*, _session_impl_t>>();

				// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
				m_acceptor_ptr = std::make_shared<boost::asio::ip::tcp::acceptor>(m_strand_ptr->get_io_service());

				// parse address and port
				boost::asio::ip::tcp::resolver resolver(m_acceptor_ptr->get_io_service());
				boost::asio::ip::tcp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::tcp::endpoint bind_endpoint = *resolver.resolve(query);

				m_acceptor_ptr->open(bind_endpoint.protocol());

				// when you close socket in linux system,and start socket immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct already before,why does this happen? the reasion is 
				// the socket option "TIME_WAIT",although you close the socket,but the system not release the socket,util 2~4 
				// seconds later,so we can use the SO_REUSEADDR option to avoid this problem,like below
				m_acceptor_ptr->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true)); // set port reuse
				m_acceptor_ptr->bind(bind_endpoint);

				m_acceptor_ptr->listen(boost::asio::socket_base::max_connections);

				_fire_listen();

				_post_accept(shared_from_this());
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());

				m_acceptor_ptr.reset();
			}

			return (m_acceptor_ptr && m_acceptor_ptr->is_open());
		}

		/**
		 * @function : stop acceptor
		 */
		virtual void stop() override
		{
			// call acceptor's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
			// can get notify to exit
			if (m_acceptor_ptr && m_acceptor_ptr->is_open())
			{
				// use promise and future to wait for the async post finished.
				std::promise<void> promise_accept;

				// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
				// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
				// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
				m_strand_ptr->post([this, &promise_accept]()
				{
					boost::system::error_code ec;

					m_acceptor_ptr->cancel(ec);
					if (ec)
						set_last_error(ec.value());

					m_acceptor_ptr->close(ec);
					if (ec)
						set_last_error(ec.value());

					auto_promise<void> ap(promise_accept);
				});

				// wait util the socket is closed completed
				// must wait for the socket closed completed,otherwise when use m_strand_ptr post a event to close the socket,
				// before socket closed ,the event thread join is returned already,and it will cause memory leaks
				// also can don't wati for the acceptor closed completed,because the _handle_accept hold the session shared_ptr,
				// and session mgr's destroy function will wait for all session destroyed,so after _handle_accept finished,
				// the session mgr's destroy can be return,so we don't need to wait for the _handle_accept return.

				// wait for the async task finish,when finished,the socket must be closed already.
				promise_accept.get_future().wait();
			}

			// first stop all connected sessions,then destroy(delete) the session pointer
			if (m_session_mgr_ptr)
			{
				// stop all the sessions, the session::stop must be no blocking,otherwise it may be cause loop lock.
				m_session_mgr_ptr->for_each_session([](std::shared_ptr<session_impl_t> session_ptr)
				{
					session_ptr->stop();
				});

				m_session_mgr_ptr->destroy();
			}
		}

		/**
		 * @function : test whether the acceptor is opened
		 */
		virtual bool is_start() override
		{
			return (m_acceptor_ptr && m_acceptor_ptr->is_open());
		}

		/**
		 * @function : get the listen address
		 */
		virtual std::string get_listen_address() override
		{
			try
			{
				if (m_acceptor_ptr && m_acceptor_ptr->is_open())
				{
					auto endpoint = m_acceptor_ptr->local_endpoint();
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
		 * @function : get the listen port
		 */
		virtual unsigned short get_listen_port() override
		{
			try
			{
				if (m_acceptor_ptr && m_acceptor_ptr->is_open())
				{
					auto endpoint = m_acceptor_ptr->local_endpoint();
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
		 * @function : get the acceptor shared_ptr
		 */
		inline std::shared_ptr<boost::asio::ip::tcp::acceptor> get_acceptor_ptr()
		{
			return m_acceptor_ptr;
		}

		/**
		 * @function : get the session manager shared_ptr
		 */
		inline std::shared_ptr<session_mgr_t<_session_impl_t*, _session_impl_t>> get_session_mgr_ptr()
		{
			return m_session_mgr_ptr;
		}


	protected:

		virtual std::shared_ptr<_session_impl_t> _prepare_session()
		{
			// get a session shared_ptr from session manager
			try
			{
				// the params of get_session is final passed to session constructor
				std::shared_ptr<_session_impl_t> session_ptr = m_session_mgr_ptr->get_session(
					m_ioservice_pool_ptr->get_io_service_ptr(),
					m_listener_mgr_ptr,
					m_url_parser_ptr,
					m_send_buf_pool_ptr,
					m_recv_buf_pool_ptr
				);

				// reset the session's status to default 
				session_ptr->_reset();

				return session_ptr;
			}
			// handle exception,may be is the exception "Too many open files" (exception code : 24)
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());

				PRINT_EXCEPTION;
			}

			return nullptr;
		}

		virtual void _post_accept(std::shared_ptr<acceptor_impl> this_ptr)
		{
			auto session_ptr = _prepare_session();

			_do_accept(this_ptr, session_ptr);
		}

		virtual void _do_accept(std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<_session_impl_t> session_ptr)
		{
			if (is_start())
			{
				auto socket_ptr = session_ptr->get_socket_ptr();
				if (session_ptr && socket_ptr)
				{
					m_acceptor_ptr->async_accept(
#ifdef USE_SSL
						socket_ptr->lowest_layer(),
#else
						*socket_ptr,
#endif
						m_strand_ptr->wrap(std::bind(&tcp_acceptor_impl::_handle_accept, std::static_pointer_cast<tcp_acceptor_impl>(this_ptr),
							std::placeholders::_1, // error_code
							this_ptr,
							session_ptr
						)));
				}
				// occur exception,may be is the exception "Too many open files" (exception code : 24)
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					m_strand_ptr->post(std::bind(&tcp_acceptor_impl::_post_accept, std::static_pointer_cast<tcp_acceptor_impl>(this_ptr), this_ptr));
				}
			}
		}

		virtual void _handle_accept(const boost::system::error_code& ec, std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<_session_impl_t> session_ptr)
		{
			set_last_error(ec.value());

			if (!ec)
			{
				_fire_accept(session_ptr);

				if (session_ptr->start())
				{
					m_session_mgr_ptr->put_session(session_ptr);
				}
			}
			else
			{
				// may be user pressed the CTRL + C to exit application.
				// the acceptor status,if closed,don't call _post_accept again.
				if (ec == boost::asio::error::operation_aborted)
				{
					_fire_shutdown(ec.value());
					return;
				}
				else if (ec != boost::asio::error::connection_reset)
				{
					PRINT_EXCEPTION;
				}

				if (!m_acceptor_ptr || !m_acceptor_ptr->is_open())
				{
					_fire_shutdown(ec.value());
					return;
				}
			}

			_post_accept(this_ptr);
		}

		virtual void _fire_listen()
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_listen();
		}

		virtual void _fire_accept(std::shared_ptr<_session_impl_t> session_ptr)
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_accept(session_ptr);
		}

		virtual void _fire_shutdown(int error)
		{
			std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_shutdown(error);
		}

	protected:
		
		/// the io_service_pool for socket event
		io_service_pool_ptr m_ioservice_pool_ptr;

		/// send buffer pool
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// recv buffer pool for every session
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// the connected session manager
		std::shared_ptr<session_mgr_t<_session_impl_t*, _session_impl_t>> m_session_mgr_ptr;
		//std::shared_ptr<tcp_session_mgr> m_session_mgr_ptr;

		/// acceptor to accept client connection
		std::shared_ptr<boost::asio::ip::tcp::acceptor> m_acceptor_ptr;

	};


}

#endif // !__ASIO2_TCP_ACCEPTOR_IMPL_HPP__
