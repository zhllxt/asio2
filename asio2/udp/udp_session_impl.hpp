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

	template<class _pool_t>
	class udp_session_impl : public session_impl
	{
		template<class _session_impl_t> friend class udp_acceptor_impl;

		template<class _Kty, class _Session, class _Hasher, class _Keyeq> friend class session_mgr_t;

	public:

		typedef _pool_t pool_t;

		/**
		 * @construct
		 */
		explicit udp_session_impl(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr,
			std::shared_ptr<pool_s> send_buf_pool_ptr,
			std::shared_ptr<pool_t> recv_buf_pool_ptr,
			volatile bool & acceptor_stop_is_called,
			int & seted_send_buffer_size,
			int & seted_recv_buffer_size
		)
			: session_impl(nullptr, listener_mgr_ptr, url_parser_ptr)
			, m_send_buf_pool_ptr(send_buf_pool_ptr)
			, m_recv_buf_pool_ptr(recv_buf_pool_ptr)
			, m_acceptor_stop_is_called(acceptor_stop_is_called)
			, m_seted_send_buffer_size(seted_send_buffer_size)
			, m_seted_recv_buffer_size(seted_recv_buffer_size)
			, m_timer(*ioservice_ptr)
			, m_timer_strand(*ioservice_ptr)
		{
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
			// user has called stop in the listener function,so we can't start continue.
			if (m_stop_is_called || m_acceptor_stop_is_called)
				return false;

			// reset the variable to default status
			m_fire_close_is_called.clear(std::memory_order_release);

			// first call base class start function
			if (!session_impl::start())
				return false;

			// when m_is_stoped is the default value,which is false,we can start this session.
			// if user call stop function in the listener function already,m_is_stoped will be set to true,
			// this session shared_ptr should be disappeared,and we should't start continue.
			if (is_start())
			{
				// set the silence timeout from url parser
				_set_silence_timeout_from_url();

				auto this_ptr = shared_from_this();
				// start the timer of check silence timeout,this timer has another purpose : when this udp session is created,
				// if has't pass a shared_from_this object to the io_service worker,will cause this udp session shared_ptr disapperd
				// immediately,and the udp session_mgr will never has any udp session object.then if we use this timer,and pass
				// the shared_from_this object to the timer's io_service worker,so this udp session shared_ptr can alived.
				m_timer.expires_from_now(boost::posix_time::seconds(m_silence_timeout));
				m_timer.async_wait(
					m_timer_strand.wrap(std::bind(&udp_session_impl::_handle_timeout,
						std::static_pointer_cast<udp_session_impl>(this_ptr),
						std::placeholders::_1, // error_code
						this_ptr
					)));

				// set send buffer size from url params
				_set_send_buffer_size_from_url();

				// set recv buffer size from url params
				_set_recv_buffer_size_from_url();

				return true;
			}

			return false;
		}

		/**
		 * @function : stop session
		 * note : this function must be noblocking,if it's blocking,will cause circle lock in session_mgr's function stop_all and stop
		 */
		virtual void stop() override
		{
			m_stop_is_called = true;

			try
			{
				// when call shared_from_this ,may be the shared_ptr of "this" has disappeared already,so call shared_from_this will
				// cause exception,and we should't post event again,
				auto this_ptr = std::static_pointer_cast<udp_session_impl>(shared_from_this());
				m_timer_strand.post([this_ptr]()
				{
					try
					{
						this_ptr->m_timer.cancel();
					}
					catch (boost::system::system_error & e)
					{
						set_last_error(e.code().value());
					}
				});
			}
			catch (std::exception &) {}
		}

		/**
		 * @function : whether the session is started
		 */
		virtual bool is_start() override
		{
			return (
				!m_stop_is_called &&
				!m_acceptor_stop_is_called &&
				m_socket_ptr && m_socket_ptr->is_open()
				);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> send_buf_ptr)  override
		{
			if (is_start() && send_buf_ptr)
			{
				try
				{
					auto this_ptr = shared_from_this();
					// must use strand.post to send data.why we should do it like this ? see udp_session_impl._post_send.
					m_send_strand_ptr->post(std::bind(&udp_session_impl::_post_send,
						std::static_pointer_cast<udp_session_impl>(this_ptr),
						this_ptr,
						send_buf_ptr
					));
					return true;
				}
				catch (std::exception &)
				{
				}
			}
			return false;
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len) override
		{
			if (is_start())
			{
				auto buf_ptr = this->m_send_buf_pool_ptr->get(get_power_number(len));
				std::memcpy((void *)buf_ptr->data(), (const void *)buf, len);
				buf_ptr->resize(len);
				return this->send(buf_ptr);
			}
			return false;
		}

		/**
		 * @function : get the socket shared_ptr
		 */
		inline std::shared_ptr<boost::asio::ip::udp::socket> get_socket_ptr()
		{
			return m_socket_ptr;
		}

		/**
		 * @function : get the local address
		 */
		virtual std::string get_local_address() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->local_endpoint();
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
		 * @function : get the local port
		 */
		virtual unsigned short get_local_port() override
		{
			try
			{
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					auto endpoint = m_socket_ptr->local_endpoint();
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
			return "";
		}

		/**
		 * @function : get the remote port
		 */
		virtual unsigned short get_remote_port() override
		{
			return m_remote_endpoint.port();
		}

		/**
		 * @function : set socket's recv buffer size.
		 *             when packet lost rate is high,you can set the recv buffer size to a big value to avoid it.
		 */
		virtual bool set_recv_buffer_size(int size) override
		{
			if (m_seted_recv_buffer_size == size)
				return true;
			try
			{
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::receive_buffer_size option(size);
					m_socket_ptr->set_option(option);
					m_seted_recv_buffer_size = size;
					return true;
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return false;
		}

		/**
		 * @function : set socket's send buffer size
		 */
		virtual bool set_send_buffer_size(int size) override
		{
			if (m_seted_send_buffer_size == size)
				return true;
			try
			{
				if (m_socket_ptr && m_socket_ptr->is_open())
				{
					boost::asio::socket_base::send_buffer_size option(size);
					m_socket_ptr->set_option(option);
					m_seted_send_buffer_size = size;
					return true;
				}
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
			}
			return false;
		}

		void attach(
			std::shared_ptr<io_service> ioservice_ptr,
			std::shared_ptr<boost::asio::io_service::strand> strand_ptr,
			std::shared_ptr<io_service> send_ioservice_ptr,
			std::shared_ptr<boost::asio::io_service::strand> send_strand_ptr,
			std::shared_ptr<boost::asio::ip::udp::socket> socket_ptr,
			boost::asio::ip::udp::endpoint endpoint
		)
		{
			m_ioservice_ptr = ioservice_ptr;
			m_strand_ptr = strand_ptr;
			m_send_ioservice_ptr = send_ioservice_ptr;
			m_send_strand_ptr = send_strand_ptr;
			m_socket_ptr = socket_ptr;
			m_remote_endpoint = endpoint;
		}

	protected:
		virtual void _set_silence_timeout_from_url()
		{
			// set silence_timeout from the url
			std::string str_silence_timeout = m_url_parser_ptr->get_param_value("silence_timeout");
			if (!str_silence_timeout.empty())
			{
				long silence_timeout = static_cast<long>(std::atoi(str_silence_timeout.c_str()));
				if (str_silence_timeout.find_last_of('m') != std::string::npos)
					silence_timeout *= 60;
				else if (str_silence_timeout.find_last_of('h') != std::string::npos)
					silence_timeout *= 60 * 60;
				if (silence_timeout < 1)
					silence_timeout = UDP_DEFAULT_SILENCE_TIMEOUT;
				
				m_silence_timeout = silence_timeout;
			}
		}

	protected:
		/**
		 * @function : colse the socket
		 */
		virtual void _close_socket() override
		{
			session_impl::_close_socket();

			m_stop_is_called = false;
		}

		/**
		 * @function : used for the unorder_map key
		 */
		virtual void * _get_key() override
		{
			return static_cast<void *>(&m_remote_endpoint);
		}

	protected:
		virtual void _handle_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			// every times recv data,we reset the last active time.
			reset_last_active_time();

			_fire_recv(this_ptr, recv_buf_ptr);
		}

		virtual void _handle_timeout(const boost::system::error_code& ec, std::shared_ptr<session_impl> this_ptr)
		{
			if (!ec)
			{
				// time out,this session shared_ptr will be disappear.
				// means that all shared_ptr references to the connection object will
				// disappear and the object will be destroyed automatically after this
				// handler returns. The connection class's destructor closes the socket.

				// silence duration seconds not exceed the silence timeout,post a timer event agagin to avoid this session shared_ptr
				// object disappear.
				if (get_silence_duration() <= m_silence_timeout * 1000)
				{
					m_timer.expires_from_now(boost::posix_time::seconds(m_silence_timeout));
					m_timer.async_wait(
						m_timer_strand.wrap(std::bind(&udp_session_impl::_handle_timeout,
							std::static_pointer_cast<udp_session_impl>(this_ptr),
							std::placeholders::_1,
							this_ptr
						)));
				}
				else
				{
					// silence timeout has elasped,but has't data trans,don't post a timer event again,so this session shared_ptr will
					// disappear and the object will be destroyed automatically after this handler returns.
					_fire_close(this_ptr, ec.value());
				}
			}
			else
			{
				// occur error,may be cancel is called
				_fire_close(this_ptr, ec.value());
			}
		}

		virtual void _post_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> send_buf_ptr)
		{
			if (is_start())
			{
				boost::system::error_code ec;
				m_socket_ptr->send_to(boost::asio::buffer(send_buf_ptr->data(),send_buf_ptr->size()), m_remote_endpoint, 0, ec);
				set_last_error(ec.value());
				_fire_send(this_ptr, send_buf_ptr, ec.value());

				if (ec)
				{
					PRINT_EXCEPTION;
				}
			}
		}

		virtual void _fire_recv(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> recv_buf_ptr)
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_recv(this_ptr, recv_buf_ptr);
		}

		virtual void _fire_send(std::shared_ptr<session_impl> this_ptr, std::shared_ptr<buffer<uint8_t>> send_buf_ptr, int error)
		{
			std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_send(this_ptr, send_buf_ptr, error);
		}

		virtual void _fire_close(std::shared_ptr<session_impl> this_ptr, int error)
		{
			if (!m_fire_close_is_called.test_and_set(std::memory_order_acquire))
			{
				std::static_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->notify_close(this_ptr, error);

				_close_socket();
			}
		}

	public:
		struct _hash_func // calc hash value 
		{
			std::size_t operator()(const boost::asio::ip::udp::endpoint & endpoint) const
			{
				std::hash<std::string> hasher;
				const auto & addr = endpoint.address();
				auto port = endpoint.port();
				if (addr.is_v4())
				{
					const auto & bytes = addr.to_v4().to_bytes();
					std::string s(sizeof(unsigned short) + bytes.size(), 0);
					std::memcpy((unsigned char*)s.data(), &port, sizeof(unsigned short));
					std::memcpy((unsigned char*)s.data() + sizeof(unsigned short), bytes.data(), bytes.size());
					return hasher(s);
				}
				else if (addr.is_v6())
				{
					const auto & bytes = addr.to_v6().to_bytes();
					std::string s(sizeof(unsigned short) + bytes.size(), 0);
					std::memcpy((unsigned char*)s.data(), &port, sizeof(unsigned short));
					std::memcpy((unsigned char*)s.data() + sizeof(unsigned short), bytes.data(), bytes.size());
					return hasher(s);
				}

				assert(false);
				return 0;
			}
		};

		struct _equal_func // compare func
		{
			bool operator()(const boost::asio::ip::udp::endpoint & _Left, const boost::asio::ip::udp::endpoint & _Right) const
			{
				return (_Left == _Right);
			}
		};

	protected:

		std::shared_ptr<boost::asio::ip::udp::socket> m_socket_ptr;

		/// send buffer pool
		std::shared_ptr<pool_s> m_send_buf_pool_ptr;

		/// Buffer pool used to store data received from the client. 
		std::shared_ptr<pool_t> m_recv_buf_pool_ptr;

		/// used for session_mgr's session unordered_map key
		boost::asio::ip::udp::endpoint m_remote_endpoint;

		/// hold the io_service shared_ptr,make sure the io_service is destroy after current object
		std::shared_ptr<io_service> m_send_ioservice_ptr;

		/// asio's strand to ensure asio.socket multi thread safe
		std::shared_ptr<boost::asio::io_service::strand> m_send_strand_ptr;

		/// silence timeout value,unit : seconds,if time out value is elapsed,and has't data trans,then this session will be closed.
		long m_silence_timeout = UDP_DEFAULT_SILENCE_TIMEOUT;

		/// timer for session time out
		boost::asio::deadline_timer m_timer;

		/// strand for timer to insure multi thread safe
		boost::asio::io_service::strand m_timer_strand;

		volatile bool & m_acceptor_stop_is_called;

		int & m_seted_send_buffer_size;
		int & m_seted_recv_buffer_size;

		/// use to check whether the user call session stop in the listener
		volatile bool m_stop_is_called = false;

		/// use to avoid call _fire_close twice
		std::atomic_flag m_fire_close_is_called = ATOMIC_FLAG_INIT;

	};

}

#endif // !__ASIO2_UDP_SESSION_IMPL_HPP__
