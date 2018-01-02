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

#include <asio2/base/acceptor_impl.hpp>

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

		/**
		 * @construct
		 */
		tcp_acceptor_impl(
			std::shared_ptr<url_parser>                    url_parser_ptr,
			std::shared_ptr<listener_mgr>                  listener_mgr_ptr,
			std::shared_ptr<io_context_pool>               io_context_pool_ptr
		)
			: acceptor_impl(url_parser_ptr, listener_mgr_ptr, io_context_pool_ptr)
			, m_acceptor(*m_io_context_ptr)
		{
			m_session_mgr_ptr = std::make_shared<session_mgr_t<_session_impl_t*>>();
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
			try
			{
				// parse address and port
				boost::asio::ip::tcp::resolver resolver(*m_io_context_ptr);
				boost::asio::ip::tcp::resolver::query query(m_url_parser_ptr->get_ip(), m_url_parser_ptr->get_port());
				boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

				m_acceptor.open(endpoint.protocol());

				// when you close socket in linux system,and start socket immediate,you will get like this "the address is in use",
				// and bind is failed,but i'm suer i close the socket correct already before,why does this happen? the reasion is 
				// the socket option "TIME_WAIT",although you close the socket,but the system not release the socket,util 2~4 
				// seconds later,so we can use the SO_REUSEADDR option to avoid this problem,like below
				m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true)); // set port reuse
				m_acceptor.bind(endpoint);

				m_acceptor.listen(boost::asio::socket_base::max_connections);

				_fire_listen();

				_post_accept(shared_from_this());
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());

				boost::system::error_code ec;
				m_acceptor.close(ec);
			}

			return (m_acceptor.is_open());
		}

		/**
		 * @function : stop acceptor
		 */
		virtual void stop() override
		{
			// call acceptor's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
			// can get notify to exit
			if (m_acceptor.is_open())
			{
				// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
				// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
				// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
				m_strand_ptr->post([this]()
				{
					try
					{
						_fire_shutdown(get_last_error());

						m_acceptor.cancel();
						m_acceptor.close();
					}
					catch (boost::system::system_error & e)
					{
						set_last_error(e.code().value());
					}

					// stop all the sessions, the session::stop must be no blocking,otherwise it may be cause loop lock.
					m_session_mgr_ptr->stop_all();
				});
			}
		}

		/**
		 * @function : check whether the acceptor is opened
		 */
		virtual bool is_start() override
		{
			return (m_acceptor.is_open());
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
			catch (boost::system::system_error & e)
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
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return 0;
		}

		/**
		 * @function : get the acceptor shared_ptr
		 */
		inline boost::asio::ip::tcp::acceptor & get_acceptor()
		{
			return m_acceptor;
		}

	protected:
		virtual std::shared_ptr<_session_impl_t> _make_session()
		{
			try
			{
				return std::make_shared<_session_impl_t>(
					this->m_url_parser_ptr,
					this->m_listener_mgr_ptr,
					this->m_io_context_pool_ptr->get_io_context_ptr(),
					this->m_session_mgr_ptr
					);
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
			if (is_start())
			{
				auto session_ptr = _make_session();
				if (session_ptr)
				{
#ifdef ASIO2_USE_SSL
					auto & socket = session_ptr->get_socket().lowest_layer();
#else
					auto & socket = session_ptr->get_socket();
#endif
					m_acceptor.async_accept(socket,
						m_strand_ptr->wrap(std::bind(&tcp_acceptor_impl::_handle_accept, this,
							std::placeholders::_1, // error_code
							std::move(this_ptr),
							std::move(session_ptr)
						)));
				}
				// occur exception,may be is the exception "Too many open files" (exception code : 24)
				else
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));

					m_strand_ptr->post(std::bind(&tcp_acceptor_impl::_post_accept, this, std::move(this_ptr)));
				}
			}
		}

		virtual void _handle_accept(const boost::system::error_code & ec, std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<session_impl> session_ptr)
		{
			if (is_start())
			{
				if (!ec)
				{
					session_ptr->start();
				}
				else
				{
					set_last_error(ec.value());
					// may be user pressed the CTRL + C to exit application.
					// the acceptor status,if closed,don't call _post_accept again.
					if (ec == boost::asio::error::operation_aborted)
						return;

					PRINT_EXCEPTION;
				}

				_post_accept(std::move(this_ptr));
			}
		}

		virtual void _fire_listen()
		{
			dynamic_cast<server_listener_mgr *>(this->m_listener_mgr_ptr.get())->notify_listen();
		}

		virtual void _fire_shutdown(int error)
		{
			dynamic_cast<server_listener_mgr *>(this->m_listener_mgr_ptr.get())->notify_shutdown(error);
		}

	protected:
		/// acceptor to accept client connection
		boost::asio::ip::tcp::acceptor       m_acceptor;

	};


}

#endif // !__ASIO2_TCP_ACCEPTOR_IMPL_HPP__
