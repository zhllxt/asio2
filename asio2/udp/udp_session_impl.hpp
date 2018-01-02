/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_UDP_SESSION_IMPL_HPP__
#define __ASIO2_UDP_SESSION_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


#include <memory>
#include <thread>
#include <deque>

#include <boost/asio.hpp>

#include <asio2/base/session_impl.hpp>
#include <asio2/base/session_mgr.hpp>
#include <asio2/base/listener_mgr.hpp>

namespace asio2
{

	class udp_session_impl : public session_impl
	{
		template<class _session_impl_t> friend class udp_acceptor_impl;

	public:
		/**
		 * @construct
		 */
		explicit udp_session_impl(
			std::shared_ptr<url_parser>                      url_parser_ptr,
			std::shared_ptr<listener_mgr>                    listener_mgr_ptr,
			std::shared_ptr<boost::asio::io_context>         io_context_ptr,
			std::shared_ptr<boost::asio::io_context::strand> strand_ptr,
			boost::asio::ip::udp::socket                   & socket,
			std::shared_ptr<boost::asio::io_context>         send_io_context_ptr,
			std::shared_ptr<boost::asio::io_context::strand> send_strand_ptr,
			boost::asio::ip::udp::endpoint                   endpoint,
			std::shared_ptr<session_mgr>                     session_mgr_ptr
		)
			: session_impl(url_parser_ptr, listener_mgr_ptr, io_context_ptr, session_mgr_ptr)
			, m_socket(socket)
			, m_send_ioservice_ptr(send_io_context_ptr)
			, m_send_strand_ptr(send_strand_ptr)
			, m_remote_endpoint(endpoint)
		{
			m_strand_ptr = strand_ptr;
		}

		/**
		 * @destruct
		 */
		virtual ~udp_session_impl()
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

			m_state = state::running;

			if (!this->m_session_mgr_ptr->start(self))
			{
				this->stop();
				return false;
			}

			// start the timer of check silence timeout,this timer has another purpose : when this udp session is created,
			// if has't pass a shared_from_this object to the io_context worker,will cause this udp session shared_ptr disapperd
			// immediately,and the udp session_mgr will never has any udp session object.then if we use this timer,and pass
			// the shared_from_this object to the timer's io_context worker,so this udp session shared_ptr can alived.
			m_timer.expires_from_now(boost::posix_time::seconds(m_url_parser_ptr->get_silence_timeout()));
			m_timer.async_wait(
				m_strand_ptr->wrap(std::bind(&udp_session_impl::_handle_timer, this,
					std::placeholders::_1, // error_code
					shared_from_this()
				)));

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
				m_state = state::stopping;

				try
				{
					auto self(shared_from_this());
					m_strand_ptr->post([this, self]()
					{
						auto this_ptr = std::const_pointer_cast<session_impl>(self);
						try
						{
							if (m_state == state::running)
								_fire_close(this_ptr, get_last_error());

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

		/**
		 * @function : whether the session is started
		 */
		virtual bool is_start() override
		{
			return ((m_state >= state::started) && m_socket.is_open());
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr)  override
		{
			// We must ensure that there is only one operation to send data at the same time,otherwise may be cause crash.
			if (is_start() && buf_ptr)
			{
				try
				{
					// must use strand.post to send data.why we should do it like this ? see udp_session._post_send.
					m_send_strand_ptr->post(std::bind(&udp_session_impl::_post_send, this,
						shared_from_this(),
						std::move(buf_ptr))
					);
					return true;
				}
				catch (std::exception &) {}
			}
			return false;
		}

		/**
		 * @function : get the socket shared_ptr
		 */
		inline boost::asio::ip::udp::socket & get_socket()
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
				return m_remote_endpoint.address().to_string();
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
			return m_remote_endpoint.port();
		}

	protected:
		/**
		 * @function : used for the unorder_map key
		 */
		virtual void * _get_key() override
		{
			return static_cast<void *>(&m_remote_endpoint);
		}

		virtual void _post_recv(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		{
			if (this->is_start())
			{
				_handle_recv(this_ptr, buf_ptr);
			}
		}

		virtual void _handle_recv(std::shared_ptr<session_impl> & this_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		{
			// every times recv data,we reset the last active time.
			reset_last_active_time();

			_fire_recv(this_ptr, buf_ptr);
		}

		virtual void _handle_timer(const boost::system::error_code & ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// time out,this session shared_ptr will be disappear.
				// means that all shared_ptr references to the connection object will
				// disappear and the object will be destroyed automatically after this
				// handler returns. The connection class's destructor closes the socket.

				// silence duration seconds not exceed the silence timeout,post a timer event agagin to avoid this session shared_ptr
				// object disappear.
				if (get_silence_duration() < m_url_parser_ptr->get_silence_timeout() * 1000)
				{
					auto remain = (m_url_parser_ptr->get_silence_timeout() * 1000) - get_silence_duration();
					m_timer.expires_from_now(boost::posix_time::milliseconds(remain));
					m_timer.async_wait(
						m_strand_ptr->wrap(std::bind(&udp_session_impl::_handle_timer, this,
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

		virtual void _post_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				m_socket.send_to(boost::asio::buffer((void *)buf_ptr->read_begin(), buf_ptr->size()), m_remote_endpoint, 0, ec);
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

	public:
		struct _hasher // calc hash value 
		{
			std::size_t operator()(const boost::asio::ip::udp::endpoint * endpoint) const
			{
				return asio2::string_hash((const unsigned char *)endpoint, sizeof(boost::asio::ip::udp::endpoint));
				//std::hash<std::string> hasher;
				//const auto & addr = endpoint->address();
				//auto port = endpoint->port();
				//if (addr.is_v4())
				//{
				//	const auto & bytes = addr.to_v4().to_bytes();
				//	std::string s(sizeof(unsigned short) + bytes.size(), 0);
				//	std::memcpy((unsigned char*)s.data(), &port, sizeof(unsigned short));
				//	std::memcpy((unsigned char*)s.data() + sizeof(unsigned short), bytes.data(), bytes.size());
				//	return hasher(s);
				//}
				//else if (addr.is_v6())
				//{
				//	const auto & bytes = addr.to_v6().to_bytes();
				//	std::string s(sizeof(unsigned short) + bytes.size(), 0);
				//	std::memcpy((unsigned char*)s.data(), &port, sizeof(unsigned short));
				//	std::memcpy((unsigned char*)s.data() + sizeof(unsigned short), bytes.data(), bytes.size());
				//	return hasher(s);
				//}

				//assert(false);
				//return 0;
			}
		};

		struct _equaler // compare func
		{
			bool operator()(const boost::asio::ip::udp::endpoint * a, const boost::asio::ip::udp::endpoint * b) const
			{
				return ((*a) == (*b));
			}
		};

	protected:
		/// 
		boost::asio::ip::udp::socket    & m_socket;

		/// send io_context
		std::shared_ptr<boost::asio::io_context>           m_send_ioservice_ptr;

		/// asio's strand to ensure asio.socket multi thread safe
		std::shared_ptr<boost::asio::io_context::strand>   m_send_strand_ptr;

		/// used for session_mgr's session unordered_map key
		boost::asio::ip::udp::endpoint                     m_remote_endpoint;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};

}

#endif // !__ASIO2_UDP_SESSION_IMPL_HPP__
