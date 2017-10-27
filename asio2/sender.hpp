/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

 /*
  * 
  * if you find some bugs,or have any troubles or questions on using this library,please contact me.
  * 
  */

#ifndef __ASIO2_SENDER_HPP__
#define __ASIO2_SENDER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/base/error.hpp>

#include <asio2/base/url_parser.hpp>
#include <asio2/base/client_impl.hpp>
#include <asio2/base/listener_mgr.hpp>

#include <asio2/tcp/tcp_client_impl.hpp>
#include <asio2/tcp/tcp_auto_client_impl.hpp>
#include <asio2/tcp/tcp_pack_client_impl.hpp>

#include <asio2/udp/udp_sender_impl.hpp>


namespace asio2
{

	/**
	 * the sender interface 
	 */
	class sender
	{
	public:
		/**
		 * @construct
		 * @param    : url - for the detailed meaning of this parameter, please refer to server::construct function
		 */
		sender(std::string url)
		{
			m_url_parser_ptr = std::make_shared<url_parser>(url);

			if /**/ (m_url_parser_ptr->get_protocol() == "udp")
				m_listener_mgr_ptr = std::make_shared<sender_listener_mgr>();
			else
				m_listener_mgr_ptr = std::make_shared<sender_listener_mgr>();

			if /**/ (m_url_parser_ptr->get_protocol() == "udp")
				m_sender_impl_ptr = std::make_shared<udp_sender_impl<pool<uint8_t>>>(m_listener_mgr_ptr, m_url_parser_ptr);
		}

		/**
		 * @destruct
		 */
		virtual ~sender()
		{
			stop();
		}

		/**
		 * @function : start the sender
		 * @param    : url - for the detailed meaning of this parameter, please refer to server::start function
		 * @return   : true - start successed , false - start failed
		 */
		bool start()
		{
			return ((m_listener_mgr_ptr && m_sender_impl_ptr) ? m_sender_impl_ptr->start() : false);
		}

		/**
		 * @function : stop the sender
		 */
		void stop()
		{
			if (m_sender_impl_ptr)
			{
				m_sender_impl_ptr->stop();
			}
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, send_buf_ptr, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, send_buf_ptr, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, const char * buf, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, const char * buf, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf, len) : false);
		}
		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, unsigned short port, const char * buf)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::string ip, std::string port, const char * buf)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf) : false);
		}

	public:

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 * must ensure the listener_sptr is valid before sender has stoped 
		 */
		template<typename _listener_t>
		sender & bind_listener(std::shared_ptr<_listener_t> listener_sptr)
		{
			try
			{
				std::dynamic_pointer_cast<sender_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_sptr);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 * must ensure the listener_rptr is valid before sender has stoped 
		 */
		template<typename _listener_t>
		sender & bind_listener(_listener_t                * listener_rptr)
		{
			try
			{
				std::dynamic_pointer_cast<sender_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_rptr);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the sender send data finished
		 * @param    : listener - a callback function like this :
		 * void on_send(std::string ip, unsigned short port, std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error);
		 */
		template<typename _listener>
		sender & bind_send(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<sender_listener_mgr>(m_listener_mgr_ptr)->bind_send(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the sender recv data from remote endpoint
		 * @param    : listener - a callback function like this :
		 * void on_recv(std::string ip, unsigned short port, std::shared_ptr<uint8_t> data_ptr, std::size_t len);
		 */
		template<typename _listener>
		sender & bind_recv(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<sender_listener_mgr>(m_listener_mgr_ptr)->bind_recv(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the sender is closed
		 * @param    : listener - a callback function like this :
		 * void on_close(int error);
		 */
		template<typename _listener>
		sender & bind_close(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<sender_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

	public:
		/**
		 * @function : get socket's recv buffer size
		 */
		inline int get_recv_buffer_size()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_recv_buffer_size() : static_cast<int>(0));
		}

		/**
		 * @function : set socket's recv buffer size.
		 *             when packet lost rate is high,you can set the recv buffer size to a big value to avoid it.
		 */
		inline bool set_recv_buffer_size(int size)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->set_recv_buffer_size(size) : false);
		}

		/**
		 * @function : get socket's send buffer size
		 */
		inline int get_send_buffer_size()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_send_buffer_size() : static_cast<int>(0));
		}

		/**
		 * @function : set socket's send buffer size
		 */
		inline bool set_send_buffer_size(int size)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->set_send_buffer_size(size) : false);
		}

		/**
		 * @function : get the local address
		 */
		inline std::string get_local_address()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_local_address() : std::string());
		}

		/**
		 * @function : get the local port
		 */
		inline unsigned short get_local_port()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_local_port() : static_cast<unsigned short>(0));
		}

		/**
		 * @function : get the remote address
		 */
		inline std::string get_remote_address()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_remote_address() : std::string());
		}

		/**
		 * @function : get the remote port
		 */
		inline unsigned short get_remote_port()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->get_remote_port() : static_cast<unsigned short>(0));
		}

	protected:

		std::shared_ptr<sender_impl>         m_sender_impl_ptr;
		std::shared_ptr<url_parser>          m_url_parser_ptr;
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;

	};
}

#endif // !__ASIO2_SENDER_HPP__
