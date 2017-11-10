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

#include <asio2/util/object_pool.hpp>

#include <asio2/http/http_request_parser.hpp>
#include <asio2/http/http_response.hpp>

#include <asio2/tcp/tcp_session_impl.hpp>

namespace asio2
{

	template<class _pool_t>
	class http_session_impl : public tcp_session_impl<_pool_t>
	{
		template<class _session_impl_t> friend class tcp_acceptor_impl;

		template<class _acceptor_impl_t> friend class http_server_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit http_session_impl(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr
		)
			: tcp_session_impl<_pool_t>(
				ioservice_ptr,
				listener_mgr_ptr,
				url_parser_ptr,
				send_buf_pool_ptr,
				recv_buf_pool_ptr
				)
			, m_keepalive_timer(*ioservice_ptr)
			, m_silence_timer(*ioservice_ptr)
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
			// reset the variable to default status
			m_request_parser.reset();
			m_request_count = 0;

			// start the keep-alive timeout timer
			auto this_ptr = shared_from_this();
			m_keepalive_timer.expires_from_now(boost::posix_time::seconds(m_keepalive_timeout));
			m_keepalive_timer.async_wait(
				m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_keepalive_timeout,
					std::static_pointer_cast<http_session_impl>(this_ptr),
					std::placeholders::_1, // error_code
					this_ptr
				)));

			// start the silence timeout timer
			m_silence_timer.expires_from_now(boost::posix_time::seconds(m_silence_timeout));
			m_silence_timer.async_wait(
				m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_silence_timeout,
					std::static_pointer_cast<http_session_impl>(this_ptr),
					std::placeholders::_1, // error_code
					this_ptr
				)));

			return tcp_session_impl<_pool_t>::start();
		}

		virtual void stop() override
		{
			tcp_session_impl<_pool_t>::stop();
		}

		http_session_impl & set_request_pool(std::shared_ptr<object_pool<http_request>> request_pool_ptr)
		{
			m_request_pool_ptr = request_pool_ptr;
			return (*this);
		}

	protected:
		virtual void _close_socket() override
		{
			tcp_session_impl<_pool_t>::_close_socket();

			try
			{
				m_keepalive_timer.cancel();
				m_silence_timer.cancel();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
		}

		virtual void _handle_keepalive_timeout(const boost::system::error_code& ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// if the session is recving data,although the timeout time has passed,we don't close the session,instead we post a event again
				// and wait for util the recving data is completed.
				if (m_request_parser.get_status() == http_request_parser::status::indeterminate)
				{
					if (get_silence_duration() < 100)
					{
						m_keepalive_timer.expires_from_now(boost::posix_time::milliseconds(m_keepalive_timeout * 10));
						m_keepalive_timer.async_wait(
							m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_keepalive_timeout,
								std::static_pointer_cast<http_session_impl>(this_ptr),
								std::placeholders::_1, // error_code
								this_ptr
							)));
					}
					// if the client send data very slowly,e.g send one byte per second,although the status is indeterminate,we also close the session
					else
					{
						_close_socket();
					}
				}
				else
				{
					_close_socket();
				}
			}
		}

		virtual void _handle_silence_timeout(const boost::system::error_code& ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// If the client connects, but doesn't send data always,the session will be closed.
				if ((get_silence_duration() + 100) >= (m_silence_timeout * 1000))
				{
					_close_socket();
				}
			}
		}

		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr) override
		{
			_post_recv(this_ptr, nullptr);
		}

		virtual void _post_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<http_request> request_ptr)
		{
			if (is_start())
			{
				// every times post recv event,we get the recv buffer from the buffer pool
				std::shared_ptr<buffer<uint8_t>> recv_buf_ptr = m_recv_buf_pool_ptr->get(0);

				m_socket_ptr->async_read_some(
					boost::asio::buffer(recv_buf_ptr->data(), recv_buf_ptr->capacity()),
					m_strand_ptr->wrap(std::bind(&http_session_impl::_handle_recv, std::static_pointer_cast<http_session_impl>(this_ptr),
						std::placeholders::_1, // error_code
						std::placeholders::_2, // bytes_recvd
						this_ptr,
						recv_buf_ptr,
						request_ptr
					)));
			}
		}

		virtual void _handle_recv(const boost::system::error_code& ec, std::size_t bytes_recvd, std::shared_ptr<session_impl> this_ptr, 
			std::shared_ptr<buffer<uint8_t>> recv_buf_ptr, std::shared_ptr<http_request> request_ptr)
		{
			if (!ec)
			{
				// every times recv data,we update the last active time.
				reset_last_active_time();

				recv_buf_ptr->resize(bytes_recvd);

				if (!request_ptr)
					request_ptr = m_request_pool_ptr->get();

				http_request_parser::status ret = m_request_parser.parse(recv_buf_ptr, request_ptr);
				if /**/ (ret == http_request_parser::status::success)
				{
					m_request_count++;

					std::string sss((const char *)recv_buf_ptr->data(), recv_buf_ptr->size());

					_fire_recv(this_ptr, request_ptr);

					if (get_keepalive_duration() > m_keepalive_timeout || m_request_count > m_max_request_count)
					{
						_fire_close(this_ptr, 0);

						return;
					}
				}
				else if (ret == http_request_parser::status::indeterminate || ret == http_request_parser::status::idle)
				{
					this->_post_recv(this_ptr, request_ptr);

					return;
				}
				else if (ret == http_request_parser::status::fail)
				{
					set_last_error(m_request_parser.get_http_errno());

					_fire_close(this_ptr, m_request_parser.get_http_errno());

					return;
				}

				this->_post_recv(this_ptr);
			}
			else
			{
				set_last_error(ec.value());

				_fire_close(this_ptr, ec.value());
			}

			// If an error occurs then no new asynchronous operations are started. This
			// means that all shared_ptr references to the connection object will
			// disappear and the object will be destroyed automatically after this
			// handler returns. The connection class's destructor closes the socket.
		}

		/// must override all listener functions,and cast the m_listener_mgr_ptr to http_server_listener_mgr,
		/// otherwise it will crash when these listener was called.
	protected:
		virtual void _fire_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<http_request> request_ptr)
		{
			std::static_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_recv(this_ptr, request_ptr);
		}

		virtual void _fire_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<http_response> response_ptr, int error)
		{
			std::static_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_send(this_ptr, response_ptr, error);
		}

		virtual void _fire_close(std::shared_ptr<session_impl> this_ptr, int error) override
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				std::static_pointer_cast<http_server_listener_mgr>(m_listener_mgr_ptr)->notify_close(this_ptr, error);

				_close_socket();
			}
		}

	protected:
		http_request_parser m_request_parser;

		std::shared_ptr<object_pool<http_request>> m_request_pool_ptr;

		/// timer for session keep-alive time out
		boost::asio::deadline_timer m_keepalive_timer;

		/// timer for session silence time out
		boost::asio::deadline_timer m_silence_timer;

		/// keep-alive timeout value,unit : seconds,if time out value is elapsed,then this session will be closed.
		long m_keepalive_timeout = HTTP_KEEPALIVE_TIMEOUT;

		/// silence timeout value,unit : seconds,if time out value is elapsed,then this session will be closed.
		long m_silence_timeout = HTTP_SILENCE_TIMEOUT;

		/// request times count,if the client send request more than max count,then this session will be closed
		size_t m_request_count = 0;

		size_t m_max_request_count = HTTP_MAX_REQUEST_COUNT;

	};

}

#endif // !__ASIO2_HTTP_SESSION_IMPL_HPP__
