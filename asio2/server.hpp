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

#ifndef __ASIO2_SERVER_HPP__
#define __ASIO2_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <memory>
#include <future>
#include <functional>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/base/error.hpp>

#include <asio2/base/url_parser.hpp>
#include <asio2/base/server_impl.hpp>
#include <asio2/base/listener_mgr.hpp>

#include <asio2/tcp/tcp_server_impl.hpp>
#include <asio2/tcp/tcp_pack_server_impl.hpp>

#include <asio2/udp/udp_server_impl.hpp>

#include <asio2/http/http_server_impl.hpp>

#if defined(USE_SSL)
#include <asio2/tcp/tcps_server_impl.hpp>
#include <asio2/tcp/tcps_pack_server_impl.hpp>
#endif

namespace asio2
{

	using tcp_session        = tcp_session_impl<pool<uint8_t>>;
	using tcp_acceptor       = tcp_acceptor_impl<tcp_session>;
	using tcp_server         = tcp_server_impl<tcp_acceptor>;

	using tcp_auto_session   = tcp_auto_session_impl<multi_pool<uint8_t>>;
	using tcp_auto_acceptor  = tcp_acceptor_impl<tcp_auto_session>;
	using tcp_auto_server    = tcp_server_impl<tcp_auto_acceptor>;

	using tcp_pack_session   = tcp_pack_session_impl<pool<uint8_t>>;
	using tcp_pack_acceptor  = tcp_pack_acceptor_impl<tcp_pack_session>;
	using tcp_pack_server    = tcp_pack_server_impl<tcp_pack_acceptor>;

	using udp_session        = udp_session_impl<pool<uint8_t>>;
	using udp_acceptor       = udp_acceptor_impl<udp_session>;
	using udp_server         = udp_server_impl<udp_acceptor>;

	using http_server        = http_server_impl<http_acceptor_impl<http_session_impl<pool<uint8_t>>>>;

#if defined(USE_SSL)
	using tcps_session       = tcps_session_impl<pool<uint8_t>>;
	using tcps_acceptor      = tcps_acceptor_impl<tcps_session>;
	using tcps_server        = tcps_server_impl<tcps_acceptor>;

	using tcps_auto_session  = tcps_auto_session_impl<multi_pool<uint8_t>>;
	using tcps_auto_acceptor = tcps_acceptor_impl<tcps_auto_session>;
	using tcps_auto_server   = tcps_server_impl<tcps_auto_acceptor>;

	using tcps_pack_session   = tcps_pack_session_impl<pool<uint8_t>>;
	using tcps_pack_acceptor  = tcps_pack_acceptor_impl<tcps_pack_session>;
	using tcps_pack_server    = tcps_pack_server_impl<tcps_pack_acceptor>;
#endif

	/**
	 * the server interface 
	 */
	class server
	{
	public:
		/**
		 * @construct
		 * @param    : url string,see below
		 *
		 * tcp
		 * tcp://127.0.0.1:3306/pack?notify_mode=async&send_buffer_size=16M&recv_buffer_size=16m&pool_buffer_size=1024k&io_service_pool_size=3&max_packet_size=1024&packet_header_flag=11
		 *
		 * tcps
		 * tcps://127.0.0.1:3306/auto?notify_mode=async&send_buffer_size=1024k&recv_buffer_size=1024K&pool_buffer_size=1024k&io_service_pool_size=3
		 *
		 * udp
		 * udp://127.0.0.1:3306/notify_mode=sync&send_buffer_size=1024k&recv_buffer_size=1024K&pool_buffer_size=1024k&io_service_pool_size=3&silence_timeout=60s
		 *
		 * http
		 * http://127.0.0.1:5432
		 *
		 * https
		 * https://127.0.0.1:1521
		 *
		 * rpc
		 * rpc://127.0.0.1:3306
		 *
		 * icmp
		 * icmp://127.0.0.1
		 *
		 * @return   : true - start successed , false - start failed
		 *
		 * @model explain :
		 * # auto - only useful to tcp, asio2 will help you handle the tcp sticky bag by add a 4 bytes header at the packet beginning,
		 *        and every times packet passed to your recv listener,the packet must be a completed packet,under auto model,you
		 *        need to set the max packet length,you can also set the packet header flag,if you don't set the packet header flag,
		 *        asio2 will use the default packet header flag 0b10101010,the header flag is 1~255 (1 to 255),the max packet length
		 *        is 0x00ffffff,the max_packet_size and packet_header_flag is only useful to auto model
		 * # pack - only useful to tcp, you need to set up a data format parser,when recved data,asio2 will call the parser to judge
		 *        whether the data is valid,if valid and completed,the recv listener will be notifyed.if data length is not enough
		 *        for a completed packet,the parse should return NEED_MORE_DATA,if the data is invalid,the parser should return
		 *        INVALID_DATA,if the data is valid and is a completed packet,the parser should return the completed packet length.
		 *        you must ensure the pool_buffer_size is greater than max packet length.
		 * # default model : when recved data,the recv listener will be notifyed,but the data content and data length is random,you
		 *        need judge whether the data is valid or completed youself.
		 *
		 * @param explain :
		 * # notify_mode - "async" or "sync"(default)
		 * # send_buffer_size - set the operation system socket send buffer size
		 * # recv_buffer_size - set the operation system socket recv buffer size,we can set this value with a large number to resolve
		 *        the packet loss problem
		 * # io_service_pool_size -
		 * # max_packet_size - just used for pack mode,set your data packet max length
		 * # packet_header_flag - just used for pack mode,set your data packet header flag
		 * # silence_timeout - if there has no data transfer for a long time,the session will be disconnect
		 * % note : when use ssl on windows,you need add "Crypt32.lib" to the additional libs.
		 */
		server(std::string url)
		{
			m_url_parser_ptr = std::make_shared<url_parser>(url);

			if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "tcps")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "udp")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "http")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "https")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "rpc")
				m_listener_mgr_ptr = std::make_shared<server_listener_mgr>();

			if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
			{
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					m_server_impl_ptr = std::make_shared<tcp_auto_server>(m_listener_mgr_ptr, m_url_parser_ptr);
				else if (m_url_parser_ptr->get_model() == "pack")
					m_server_impl_ptr = std::make_shared<tcp_pack_server>(m_listener_mgr_ptr, m_url_parser_ptr);
				else
					m_server_impl_ptr = std::make_shared<tcp_server>(m_listener_mgr_ptr, m_url_parser_ptr);
			}
			else if (m_url_parser_ptr->get_protocol() == "tcps")
			{
#if defined(USE_SSL)
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					m_server_impl_ptr = std::make_shared<tcps_auto_server>(m_listener_mgr_ptr, m_url_parser_ptr);
				else if (m_url_parser_ptr->get_model() == "pack")
					m_server_impl_ptr = std::make_shared<tcps_pack_server>(m_listener_mgr_ptr, m_url_parser_ptr);
				else
					m_server_impl_ptr = std::make_shared<tcps_server>(m_listener_mgr_ptr, m_url_parser_ptr);
#else
				throw std::runtime_error("you must #define USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
			}
			else if (m_url_parser_ptr->get_protocol() == "udp")
				m_server_impl_ptr = std::make_shared<udp_server>(m_listener_mgr_ptr, m_url_parser_ptr);
			else if (m_url_parser_ptr->get_protocol() == "http")
				m_server_impl_ptr = std::make_shared<http_server>(m_listener_mgr_ptr, m_url_parser_ptr);
			else if (m_url_parser_ptr->get_protocol() == "https")
				m_server_impl_ptr = nullptr;
			else if (m_url_parser_ptr->get_protocol() == "rpc")
				m_server_impl_ptr = nullptr;
		}

		/**
		 * @destruct
		 */
		virtual ~server()
		{
			stop();
		}

		/**
		 * @function : start the server
		 */
		bool start()
		{
			return ((m_listener_mgr_ptr && m_server_impl_ptr) ? m_server_impl_ptr->start() : false);
		}

		/**
		 * @function : stop the server
		 */
		void stop()
		{
			if (m_server_impl_ptr)
			{
				m_server_impl_ptr->stop();
			}
		}

		bool is_start()
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->is_start() : false);
		}

		/**
		 * @function : send data
		 * @param    : send_buf_ptr - std::shared_ptr<uint8_t> object
		 *             len          - data len
		 */
		virtual bool send(std::shared_ptr<uint8_t> send_buf_ptr, std::size_t len)
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->send(send_buf_ptr, len) : false);
		}

		/**
		 * @function : send data
		 * @param    : buf - const char pointer
		 *             len - buf len
		 */
		virtual bool send(const char * buf, std::size_t len)
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->send(buf, len) : false);
		}

		/**
		 * @function : send data, inner use std::strlen(buf) to calc the buf len.
		 * @param    : buf - const char pointer
		 */
		virtual bool send(const char * buf)
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->send(buf) : false);
		}

		/**
		 * @function : 
		 * @param    : the user callback function,the session shared_ptr will pass to the function as a param,
		 *             the callback like this : void handler(std::shared_ptr<asio2::session> session_ptr){...}
		 */
		template<typename _handler>
		bool for_each_session(_handler handler)
		{
			if (!m_server_impl_ptr)
				return false;

			if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
			{
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					return std::dynamic_pointer_cast<tcp_auto_server>(m_server_impl_ptr)->for_each_session(handler);
				else if (m_url_parser_ptr->get_model() == "pack")
					return std::dynamic_pointer_cast<tcp_pack_server>(m_server_impl_ptr)->for_each_session(handler);
				else
					return std::dynamic_pointer_cast<tcp_server>(m_server_impl_ptr)->for_each_session(handler);
			}
			else if (m_url_parser_ptr->get_protocol() == "tcps")
			{
#if defined(USE_SSL)
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					return std::dynamic_pointer_cast<tcps_auto_server>(m_server_impl_ptr)->for_each_session(handler);
				else if (m_url_parser_ptr->get_model() == "pack")
					return std::dynamic_pointer_cast<tcps_pack_server>(m_server_impl_ptr)->for_each_session(handler);
				else
					return std::dynamic_pointer_cast<tcps_server>(m_server_impl_ptr)->for_each_session(handler);
#else
				throw std::runtime_error("you must #define USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
			}
			else if (m_url_parser_ptr->get_protocol() == "udp")
				return std::dynamic_pointer_cast<udp_server>(m_server_impl_ptr)->for_each_session(handler);
			else if (m_url_parser_ptr->get_protocol() == "http")
				return std::dynamic_pointer_cast<http_server>(m_server_impl_ptr)->for_each_session(handler);
			else if (m_url_parser_ptr->get_protocol() == "https")
				m_server_impl_ptr = nullptr;
			else if (m_url_parser_ptr->get_protocol() == "rpc")
				m_server_impl_ptr = nullptr;

			return false;
		}

		/**
		 * @function : get the listen address
		 */
		std::string get_listen_address()
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->get_listen_address() : std::string());
		}

		/**
		 * @function : get the listen port
		 */
		unsigned short get_listen_port()
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->get_listen_port() : static_cast<unsigned short>(0));
		}

		/**
		 * @function : get send pending packet size
		 */
		std::size_t get_send_pending()
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->get_send_pending() : static_cast<std::size_t>(0));
		}

		/**
		 * @function : get connected session count
		 */
		std::size_t get_session_count()
		{
			return (m_server_impl_ptr ? m_server_impl_ptr->get_session_count() : 0);
		}

	public:
		/**
		 * @function : set the data parser under pack model,you must call set_pack_parser before call server::start()
		 */
		template<typename _parser>
		server & set_pack_parser(_parser parser)
		{
			try
			{
				if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
				{
					std::dynamic_pointer_cast<tcp_pack_server>(m_server_impl_ptr)->set_pack_parser(parser);
				}
				else if (m_url_parser_ptr->get_protocol() == "tcps")
				{
#if defined(USE_SSL)
					std::dynamic_pointer_cast<tcps_pack_server>(m_server_impl_ptr)->set_pack_parser(parser);
#else
					throw std::runtime_error("you must #define USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
				}
			}
			catch (std::exception &) {}
			return (*this);
		}

#if defined(USE_SSL)
	public:
		server & use_certificate(std::string buffer)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_certificate(
					boost::asio::const_buffer(buffer.c_str(), buffer.length()), boost::asio::ssl::context::pem);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_certificate_file(std::string file)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_certificate_file(
					file, boost::asio::ssl::context::pem);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_certificate_chain(std::string buffer)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_certificate_chain(
					boost::asio::const_buffer(buffer.c_str(), buffer.length()));
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_certificate_chain_file(std::string file)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_certificate_chain_file(file);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_private_key(std::string buffer)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_private_key(
					boost::asio::const_buffer(buffer.c_str(), buffer.length()), boost::asio::ssl::context::pem);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_private_key_file(std::string file)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_private_key_file(
					file, boost::asio::ssl::context::pem);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_tmp_dh(std::string buffer)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_tmp_dh(
					boost::asio::const_buffer(buffer.c_str(), buffer.length()));
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & use_tmp_dh_file(std::string file)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->get_context_ptr()->use_tmp_dh_file(file);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
		server & set_password(std::string password)
		{
			try
			{
				std::static_pointer_cast<tcps_server>(m_server_impl_ptr)->set_password(password);
			}
			catch (std::exception & e)
			{
				set_last_error(DEFAULT_EXCEPTION_CODE, e.what());
				PRINT_EXCEPTION;
			}
			return (*this);
		}
#endif

	public:
		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 * must ensure the listener_sptr is valid before server has stoped 
		 */
		template<typename _listener_t>
		server & bind_listener(std::shared_ptr<_listener_t> listener_sptr)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_sptr);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 * must ensure the listener_rptr is valid before server has stoped 
		 */
		template<typename _listener_t>
		server & bind_listener(_listener_t                * listener_rptr)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_rptr);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the session send data finished
		 * @param    : listener - a callback function
		 * the callback function like this :
		 * void on_send(std::shared_ptr<asio2::session> session_ptr, std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error)
		 * or a lumbda function like this :
		 * [&](std::shared_ptr<asio2::session> session_ptr, std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error){}
		 */
		template<typename _listener>
		server & bind_send(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_send(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the session recv data from remote endpoint
		 * @param    : listener - a callback function like this:
		 * void on_recv(std::shared_ptr<asio2::session> session_ptr, std::shared_ptr<uint8_t> data_ptr, std::size_t len)
		 */
		template<typename _listener>
		server & bind_recv(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_recv(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the server call "listen" successed
		 * @param    : listener - a callback function like this:
		 * void on_listen()
		 */
		template<typename _listener>
		server & bind_listen(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_listen(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - accept a new session
		 * @param    : listener - a callback function like this:
		 * void on_accept(std::shared_ptr<asio2::session> session_ptr)
		 */
		template<typename _listener>
		server & bind_accept(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_accept(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the session is closed
		 * @param    : listener - a callback function like this:
		 * void on_close(std::shared_ptr<asio2::session> session_ptr, int error)
		 */
		template<typename _listener>
		server & bind_close(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

		/**
		 * @function : bind listener - the server is shutdown
		 * @param    : listener - a callback function like this:
		 * void on_shutdown(int error)
		 */
		template<typename _listener>
		server & bind_shutdown(_listener listener)
		{
			try
			{
				std::dynamic_pointer_cast<server_listener_mgr>(m_listener_mgr_ptr)->bind_shutdown(listener);
			}
			catch (std::exception &) {}
			return (*this);
		}

	protected:

		using parser_callback = std::size_t(std::shared_ptr<uint8_t> data_ptr, std::size_t len);

		std::shared_ptr<server_impl>         m_server_impl_ptr;
		std::shared_ptr<url_parser>          m_url_parser_ptr;
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;

	};
}

#endif // !__ASIO2_SERVER_HPP__
