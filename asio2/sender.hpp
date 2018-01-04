/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_SENDER_HPP__
#define __ASIO2_SENDER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <asio2/udp/udp_sender_impl.hpp>

namespace asio2
{
	using udp_sender         = udp_sender_impl;

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
				m_sender_impl_ptr = std::make_shared<udp_sender>(m_url_parser_ptr, m_listener_mgr_ptr);
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

		bool is_start()
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->is_start() : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, std::move(buf_ptr)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, std::move(buf_ptr)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, const uint8_t * buf, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf, len) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, const uint8_t * buf, std::size_t len)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf, len) : false);
		}
		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, unsigned short port, const char * buf)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const std::string & ip, const std::string & port, const char * buf)
		{
			return (m_sender_impl_ptr ? m_sender_impl_ptr->send(ip, port, buf) : false);
		}

	public:
		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 * must ensure the listener_sptr is valid before sender has stoped 
		 */
		sender & bind_listener(std::shared_ptr<sender_listener> listener_sptr)
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
		sender & bind_listener(sender_listener                * listener_rptr)
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
		 * void on_send(const std::string & ip, unsigned short port, asio2::buffer_ptr & buf_ptr, int error);
		 */
		sender & bind_send(std::function<sender_listener_mgr::send_callback> listener)
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
		 * void on_recv(const std::string & ip, unsigned short port, asio2::buffer_ptr & buf_ptr);
		 */
		sender & bind_recv(std::function<sender_listener_mgr::recv_callback> listener)
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
		sender & bind_close(std::function<sender_listener_mgr::clos_callback> listener)
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
		std::shared_ptr<url_parser  >        m_url_parser_ptr;
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;
		std::shared_ptr<sender_impl >        m_sender_impl_ptr;

	};
}

#endif // !__ASIO2_SENDER_HPP__
