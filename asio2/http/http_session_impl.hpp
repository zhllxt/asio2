/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * http://blog.csdn.net/zzhongcy/article/details/41981855
 */

#ifndef __ASIO2_HTTP_SESSION_IMPL_HPP__
#define __ASIO2_HTTP_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/http/http_request_parser.hpp>
#include <asio2/http/http_response.hpp>

#include <asio2/tcp/tcp_session_impl.hpp>

namespace asio2
{

	class http_session_impl : public tcp_session_impl
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _acceptor_impl_t> friend class http_server_impl;

	public:
		/**
		 * @construct
		 */
		explicit http_session_impl(
			std::shared_ptr<url_parser>             url_parser_ptr,
			std::shared_ptr<listener_mgr>           listener_mgr_ptr,
			std::shared_ptr<asio::io_context>       io_context_ptr,
			std::shared_ptr<session_mgr>            session_mgr_ptr
		)
			: tcp_session_impl(
				url_parser_ptr,
				listener_mgr_ptr,
				io_context_ptr,
				session_mgr_ptr
			)
			, m_keepalive_timer(*io_context_ptr)
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_session_impl()
		{
		}

		virtual bool start() override
		{
			m_state = state::starting;

			// reset the variable to default status
			m_request_parser.reset();
			m_request_count = 0;

			if (tcp_session_impl::start())
			{
				// start the keep-alive timeout timer
				m_keepalive_timer.expires_from_now(std::chrono::seconds(m_keepalive_timeout));
				m_keepalive_timer.async_wait(
					m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_keepalive_timeout, this,
						std::placeholders::_1, // error_code
						shared_from_this()
					)));
			}

			return is_started();
		}

		virtual void stop() override
		{
			tcp_session_impl::stop();

			try
			{
				m_keepalive_timer.cancel();
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
			}
		}

	protected:
		virtual void _handle_keepalive_timeout(const asio::error_code& ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// if the session is recving data,although the timeout time has passed,we don't close the session,instead we post a event again
				// and wait for util the recving data is completed.
				if (m_request_parser.get_status() == http_request_parser::status::indeterminate)
				{
					if (get_silence_duration() < 100)
					{
						m_keepalive_timer.expires_from_now(std::chrono::milliseconds(m_keepalive_timeout * 10));
						m_keepalive_timer.async_wait(
							m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_keepalive_timeout, this,
								std::placeholders::_1, // error_code
								std::move(this_ptr)
							)));
					}
					// if the client send data very slowly,e.g send one byte per second,although the status is indeterminate,we also close the session
					else
					{
						this->stop();
					}
				}
				else
				{
					this->stop();
				}
			}
		}

		virtual void _handle_recv(const asio::error_code & ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr) override
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				buf_ptr->write_bytes(bytes_recvd);

				if (!m_request_ptr)
					m_request_ptr = std::make_shared<http_request>();

				//http_request_parser::status ret = m_request_parser.parse(buf_ptr, m_request_ptr);
				//if /**/ (ret == http_request_parser::status::success)
				//{
				//	m_request_count++;

				//	std::string sss((const char *)buf_ptr->read_begin(), buf_ptr->size());

				//	_fire_recv(this_ptr, m_request_ptr);

				//	if (get_connect_duration() > m_keepalive_timeout || m_request_count > m_max_request_count)
				//	{
				//		this->stop();

				//		return;
				//	}
				//}
				//else if (ret == http_request_parser::status::indeterminate || ret == http_request_parser::status::idle)
				//{
				//	this->_post_recv(this_ptr);

				//	return;
				//}
				//else if (ret == http_request_parser::status::fail)
				//{
				//	set_last_error(m_request_parser.get_http_errno());

				//	this->stop();

				//	return;
				//}

				//this->_post_recv(this_ptr);
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

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_accept(std::shared_ptr<session_impl> & this_ptr)
		{
			static_cast<http_server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_accept(this_ptr);
		}

		virtual void _fire_recv(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<http_request> & request_ptr)
		{
			static_cast<http_server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_recv(this_ptr, request_ptr);
		}

		virtual void _fire_send(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<http_response> & response_ptr, int error)
		{
			static_cast<http_server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_send(this_ptr, response_ptr, error);
		}

		virtual void _fire_close(std::shared_ptr<session_impl> & this_ptr, int error) override
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				static_cast<http_server_listener_mgr *>(m_listener_mgr_ptr.get())->notify_close(this_ptr, error);
			}
		}

	protected:
		/// http parser
		http_request_parser           m_request_parser;

		/// request shared_ptr
		std::shared_ptr<http_request> m_request_ptr;

		/// timer for session keep-alive time out
		asio::steady_timer            m_keepalive_timer;

		/// keep-alive timeout value,unit : seconds,if time out value is elapsed,then this session will be closed.
		long   m_keepalive_timeout = ASIO2_DEFAULT_HTTP_KEEPALIVE_TIMEOUT;

		/// silence timeout value,unit : seconds,if time out value is elapsed,then this session will be closed.
		long   m_silence_timeout   = ASIO2_DEFAULT_HTTP_SILENCE_TIMEOUT;

		/// request times count,if the client send request more than max count,then this session will be closed
		size_t m_request_count     = 0;

		size_t m_max_request_count = ASIO2_DEFAULT_HTTP_MAX_REQUEST_COUNT;

	};

}

#endif // !__ASIO2_HTTP_SESSION_IMPL_HPP__
