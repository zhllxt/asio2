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

#include <memory>
#include <chrono>
#include <atomic>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/helper.hpp>

#include <asio2/base/acceptor_impl.hpp>
#include <asio2/base/session_mgr.hpp>

#include <asio2/udp/udp_session_impl.hpp>

namespace asio2
{

	template<class _session_impl_t>
	class udp_acceptor_impl : public acceptor_impl
	{
	public:

		typedef _session_impl_t session_impl_t;
		typedef typename _session_impl_t::pool_t pool_t;

		typedef typename _session_impl_t::_hash_func hasher;
		typedef typename _session_impl_t::_equal_func equaler;

		using udp_session_mgr = session_mgr_t<boost::asio::ip::udp::endpoint, _session_impl_t, hasher, equaler>;

		/**
		 * @construct
		 */
		udp_acceptor_impl(
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
		virtual ~udp_acceptor_impl()
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
				m_session_mgr_ptr = std::make_shared<udp_session_mgr>();

				boost::asio::ip::udp::endpoint bind_endpoint(
					boost::asio::ip::address::from_string(m_url_parser_ptr->get_ip()), 
					static_cast<unsigned short>(std::atoi(m_url_parser_ptr->get_port().c_str())));

				// send io_service and strand used for send data ,socket is full duplex,can read and write at the same time
				m_send_ioservice_ptr = m_ioservice_pool_ptr->get_io_service_ptr();

				m_send_strand_ptr = std::make_shared<boost::asio::io_service::strand>(*m_send_ioservice_ptr);

				// socket contructor function with endpoint param will automic call open and bind.
				m_acceptor_ptr = std::make_shared<boost::asio::ip::udp::socket>(
					*m_ioservice_ptr, bind_endpoint);

				//m_acceptor_ptr->open(bind_endpoint.protocol());
				//m_acceptor_ptr->bind(bind_endpoint);

				_fire_listen();

				_post_recv(shared_from_this());
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
			m_stop_is_called = true;

			// call listen socket's close function to notify the _handle_accept function response with error > 0 ,then the listen socket 
			// can get notify to exit
			if (m_acceptor_ptr && m_acceptor_ptr->is_open())
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
				// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
				// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
				m_strand_ptr->post([this, &promise_recv]()
				{
					boost::system::error_code ec;

					m_acceptor_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
					if (ec)
						set_last_error(ec.value());

					m_acceptor_ptr->close(ec);
					if (ec)
						set_last_error(ec.value());

					auto_promise<void> ap(promise_recv);
				});

				// wait util the socket is closed completed
				// must wait for the socket closed completed,otherwise when use m_strand_ptr post a event to close the socket,
				// before socket closed ,the event thread join is returned already,and it will cause memory leaks

				// wait for the async task finish,when finished,the socket must be closed already.
				promise_recv.get_future().wait();
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

			m_stop_is_called = false;
		}

		/**
		 * @function : test whether the acceptor is opened
		 */
		virtual bool is_start() override
		{
			return (!m_stop_is_called && m_acceptor_ptr && m_acceptor_ptr->is_open());
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
		virtual std::shared_ptr<boost::asio::ip::udp::socket> get_acceptor_ptr()
		{
			return m_acceptor_ptr;
		}

		/**
		 * @function : get the session manager shared_ptr
		 */
		inline std::shared_ptr<udp_session_mgr> get_session_mgr_ptr()
		{
			return m_session_mgr_ptr;
		}

	protected:

		virtual void _post_recv(std::shared_ptr<acceptor_impl> this_ptr)
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				try
				{
					m_acceptor_ptr->async_receive_from(
						boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
						m_sender_endpoint,
						m_strand_ptr->wrap(std::bind(&udp_acceptor_impl::_handle_recv, std::static_pointer_cast<udp_acceptor_impl>(this_ptr),
							std::placeholders::_1, // error_code
							std::placeholders::_2, // bytes_recvd
							this_ptr,
							recv_buf_ptr
						)));
				}
				catch (std::exception &)
				{
				}
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<acceptor_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
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

				if (is_start())
				{
					// first we find whether the session is in the session_mgr pool already,if not ,we new a session and put it into the session_mgr
					// pool
					std::shared_ptr<_session_impl_t> session_ptr = m_session_mgr_ptr->find_session(m_sender_endpoint);

					if (!session_ptr)
					{
						session_ptr = _handle_accept();
					}

					if (session_ptr)
					{
						session_ptr->_handle_recv(session_ptr, recv_buf_ptr);
					}
				}
			}
			else
			{
				set_last_error(ec.value());

				// may be user pressed the CTRL + C to exit application.
				// if user call stop to stop server,the socket is closed,then _handle_recv will be called,and with error,so when appear error,we check 
				// the socket status,if closed,don't _post_recv again.
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
				//else if (ec == boost::asio::error::connection_refused)
				//{
				//	// in some situations,code will run to here,but the m_sender_endpoint's ip is all 0,so can't find the udp_session to erase.
				//	boost::asio::ip::udp::endpoint sender_endpoint = m_sender_endpoint;

				//	io_service_acceptor.post(std::bind(&udp_server::_handle_close,
				//		std::static_pointer_cast<udp_server>(shared_from_this()),
				//		sender_endpoint));
				//}
				//else if (ec == boost::asio::error::connection_aborted)
				//{
				//}
			}

			// continue post next recv request.
			_post_recv(this_ptr);
		}

		virtual std::shared_ptr<_session_impl_t> _prepare_session()
		{
			try
			{
				std::shared_ptr<_session_impl_t> session_ptr = m_session_mgr_ptr->get_session(
					m_ioservice_pool_ptr->get_io_service_ptr(),
					m_listener_mgr_ptr,
					m_url_parser_ptr,
					m_send_buf_pool_ptr,
					m_recv_buf_pool_ptr,
					m_stop_is_called,
					m_seted_send_buffer_size,
					m_seted_recv_buffer_size
				);

				// must reset the session's resource 
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

		virtual std::shared_ptr<_session_impl_t> _handle_accept()
		{
			std::shared_ptr<_session_impl_t> session_ptr = _prepare_session();

			if (session_ptr)
			{
				// attach items
				session_ptr->attach(
					m_ioservice_ptr,
					m_strand_ptr,
					m_send_ioservice_ptr,
					m_send_strand_ptr,
					m_acceptor_ptr,
					m_sender_endpoint
				);

				_fire_accept(session_ptr);

				if (session_ptr->start())
				{
					m_session_mgr_ptr->put_session(session_ptr);
				}
			}

			return session_ptr;
		}

		virtual void _fire_listen()
		{
			std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_listen();
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
		std::shared_ptr<udp_session_mgr> m_session_mgr_ptr;

		/// acceptor to accept client connection
		std::shared_ptr<boost::asio::ip::udp::socket> m_acceptor_ptr;

		/// endpoint for udp 
		boost::asio::ip::udp::endpoint m_sender_endpoint;

		/// hold the io_service shared_ptr,make sure the io_service is destroy after current object
		std::shared_ptr<io_service> m_send_ioservice_ptr;

		/// asio's strand to ensure asio.socket multi thread safe
		std::shared_ptr<boost::asio::io_service::strand> m_send_strand_ptr;

		/// use to check whether the user call session stop function
		volatile bool m_stop_is_called = false;

		int m_seted_send_buffer_size = 0;
		int m_seted_recv_buffer_size = 0;

	};

}

#endif // !__ASIO2_UDP_ACCEPTOR_IMPL_HPP__
