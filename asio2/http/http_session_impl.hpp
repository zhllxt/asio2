/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_SESSION_IMPL_HPP__
#define __ASIO2_HTTP_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_parser.h>

#include <asio2/tcp/tcp_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class http_session_impl : public tcp_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 * @param    : io_service_evt - the io_service used to handle the socket event
		 *           : io_service_msg - the io_service used to handle the received msg
		 */
		explicit http_session_impl(
			io_service_ptr evt_ioservice_ptr,
			io_service_ptr msg_ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<_pool_t> recv_buf_pool_ptr
		)
			: tcp_session_impl<_pool_t>(
				evt_ioservice_ptr,
				msg_ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				recv_buf_pool_ptr
				)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_session_impl()
		{
		}

	//	/**
	//	 * @function : start this session for prepare to recv msg
	//	 */
	//	virtual bool start() override
	//	{
	//		// user has called stop in the listener function,so we can't start continue.
	//		if (m_stop_is_called)
	//			return false;

	//		// first call base class start function
	//		if (!tcp_session_impl<_pool_t>::start())
	//			return false;

	//		if (is_start())
	//		{
	//			m_async_notify = (m_url_parser_ptr->get_param_value("notify_mode") == "async");

	//			// set keeplive
	//			set_keepalive_vals();

	//			// set send buffer size from url params
	//			_set_send_buffer_size_from_url();

	//			// set recv buffer size from url params
	//			_set_recv_buffer_size_from_url();

	//			_post_recv();

	//			return true;
	//		}

	//		return false;
	//	}

	//	/**
	//	 * @function : stop session
	//	 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
	//	 */
	//	virtual void stop() override
	//	{
	//		bool is_start = this->is_start();

	//		m_stop_is_called = true;

	//		// call socket's close function to notify the _handle_recv function response with error > 0 ,then the socket 
	//		// can get notify to exit
	//		if (m_socket_ptr && m_socket_ptr->is_open())
	//		{
	//			// close the socket by post a event
	//			if (is_start)
	//			{
	//				// asio don't allow operate the same socket in multi thread,if you close socket in one thread and another thread is 
	//				// calling socket's async_... function,it will crash.so we must care for operate the socket.when need close the
	//				// socket ,we use the strand to post a event,make sure the socket's close operation is in the same thread.
	//				try
	//				{
	//					// when call shared_from_this ,may be the shared_ptr of "this" has disappeared already,so call shared_from_this will
	//					// cause exception,and we should't post event again,
	//					auto this_ptr = std::static_pointer_cast<http_session_impl>(shared_from_this());
	//					m_evt_strand_ptr->post([this_ptr]()
	//					{
	//						boost::system::error_code ec;

	//						this_ptr->m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
	//						if (ec)
	//							set_last_error(ec.value(), ec.message());

	//						this_ptr->m_socket_ptr->close(ec);
	//						if (ec)
	//							set_last_error(ec.value(), ec.message());
	//					});
	//				}
	//				catch (std::exception &) {}
	//			}
	//		}
	//	}

	//	/**
	//	 * @function : whether the session is started
	//	 */
	//	virtual bool is_start() override
	//	{
	//		return (
	//			!m_stop_is_called &&
	//			m_socket_ptr && m_socket_ptr->is_open() 
	//			);
	//	}

	//	/**
	//	 * @function : send data
	//	 */
	//	virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
	//	{
	//		if (is_start())
	//		{
	//			try
	//			{
	//				// note : can't use m_msg_ioservice_ptr to post event,because can't operate socket in multi thread.
	//				// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
	//				m_evt_strand_ptr->post(std::bind(&http_session_impl::_post_send,
	//					std::static_pointer_cast<http_session_impl>(shared_from_this()),
	//					send_buf_ptr,
	//					len
	//				));
	//				return true;
	//			}
	//			catch (std::exception & e)
	//			{
	//				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
	//				PRINT_EXCEPTION;
	//			}
	//		}
	//		return false;
	//	}

	//protected:
	//	/**
	//	 * @function : reset the resource to default status
	//	 */
	//	virtual void _reset() override
	//	{
	//		tcp_session_impl<_pool_t>::_reset();

	//		if (m_socket_ptr && m_socket_ptr->is_open())
	//		{
	//			boost::system::error_code ec;

	//			m_socket_ptr->shutdown(boost::asio::socket_base::shutdown_both, ec);
	//			if (ec)
	//				set_last_error(ec.value(), ec.message());

	//			m_socket_ptr->close(ec);
	//			if (ec)
	//				set_last_error(ec.value(), ec.message());
	//		}
	//		
	//		m_async_notify = false;

	//		m_stop_is_called = false;
	//		m_fire_close_is_called.clear(std::memory_order_release);
	//	}

	//	virtual void * _get_key() override
	//	{
	//		return static_cast<void *>(&_key);
	//	}

	//	virtual void _post_recv() override
	//	{
	//		if (is_start())
	//		{
	//			// every times post recv event,we get the recv buffer from the buffer pool
	//			std::shared_ptr<uint8_t> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

	//			m_socket_ptr->async_read_some(
	//				boost::asio::buffer(recv_buf_ptr.get(), m_recv_buf_pool_ptr->get_requested_size()),
	//				m_evt_strand_ptr->wrap(std::bind(&http_session_impl::_handle_recv, std::static_pointer_cast<http_session_impl>(shared_from_this()),
	//					std::placeholders::_1,
	//					std::placeholders::_2,
	//					recv_buf_ptr
	//				)));
	//		}
	//	}

	//	virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<uint8_t> recv_buf_ptr) override
	//	{
	//		set_last_error(ec.value(), ec.message());

	//		// every times recv data,we update the last active time.
	//		reset_last_active_time();

	//		if (!ec)
	//		{
	//			_fire_recv(recv_buf_ptr, bytes_recvd);

	//			_post_recv();
	//		}
	//		else
	//		{
	//			_fire_close(ec.value());
	//		}

	//		// If an error occurs then no new asynchronous operations are started. This
	//		// means that all shared_ptr references to the connection object will
	//		// disappear and the object will be destroyed automatically after this
	//		// handler returns. The connection class's destructor closes the socket.
	//	}

	//	virtual void _post_send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len) override
	//	{
	//		if (is_start())
	//		{
	//			boost::system::error_code ec;
	//			size_t bytes_sent = boost::asio::write(*m_socket_ptr, boost::asio::buffer(send_buf_ptr.get(), len), ec);
	//			set_last_error(ec.value(), ec.message());
	//			_fire_send(send_buf_ptr, bytes_sent, ec.value());

	//			if (ec)
	//			{
	//				PRINT_EXCEPTION;

	//				_fire_close(ec.value());
	//			}
	//		}
	//	}


	protected:

	};

}

#endif // !__ASIO2_HTTP_SESSION_IMPL_HPP__
