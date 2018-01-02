/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 *
 */

#ifndef __ASIO2_CLIENT_HPP__
#define __ASIO2_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <asio2/tcp/tcp_client_impl.hpp>
#include <asio2/tcp/tcp_auto_client_impl.hpp>
#include <asio2/tcp/tcp_pack_client_impl.hpp>

#include <asio2/udp/udp_client_impl.hpp>

#include <asio2/http/http_client_impl.hpp>

#if defined(ASIO2_USE_SSL)
#include <asio2/tcp/tcps_client_impl.hpp>
#include <asio2/tcp/tcps_auto_client_impl.hpp>
#include <asio2/tcp/tcps_pack_client_impl.hpp>
#endif


namespace asio2
{

	using tcp_connection        = tcp_connection_impl;
	using tcp_client            = tcp_client_impl<tcp_connection>;

	using tcp_auto_connection   = tcp_auto_connection_impl;
	using tcp_auto_client       = tcp_auto_client_impl<tcp_auto_connection>;

	using tcp_pack_connection   = tcp_pack_connection_impl;
	using tcp_pack_client       = tcp_pack_client_impl<tcp_pack_connection>;

	using udp_connection        = udp_connection_impl;
	using udp_client            = udp_client_impl<udp_connection>;

	using http_connection       = http_connection_impl;
	using http_client           = http_client_impl<http_connection>;

#if defined(ASIO2_USE_SSL)
	using tcps_connection       = tcps_connection_impl;
	using tcps_client           = tcps_client_impl<tcps_connection>;

	using tcps_auto_connection  = tcps_auto_connection_impl;
	using tcps_auto_client      = tcps_auto_client_impl<tcps_auto_connection>;

	using tcps_pack_connection   = tcps_pack_connection_impl;
	using tcps_pack_client       = tcps_pack_client_impl<tcps_pack_connection>;
#endif

	/**
	 * the client interface 
	 */
	class client
	{
	public:
		/**
		 * @construct
		 * @param    : url - for the detailed meaning of this parameter, please refer to server::construct function
		 */
		client(std::string url
#if defined(ASIO2_USE_SSL)
			, boost::asio::ssl::context::method  method  = boost::asio::ssl::context::sslv23
			, boost::asio::ssl::context::options options =
														   boost::asio::ssl::context::default_workarounds |
														   boost::asio::ssl::context::no_sslv2 |
														   boost::asio::ssl::context::single_dh_use
#endif
		)
		{
#if defined(ASIO2_USE_SSL)
			_init(url, method, options);
#else
			_init(url);
#endif
		}

		/**
		 * @destruct
		 */
		virtual ~client()
		{
			stop();
		}

		/**
		 * @function : start the client
		 * @param    : async_connect - asynchronous connect to the server or sync
		 * @return   : true - start successed , false - start failed
		 */
		bool start(bool async_connect = true)
		{
			return ((m_listener_mgr_ptr && m_client_impl_ptr) ? m_client_impl_ptr->start(async_connect) : false);
		}

		/**
		 * @function : stop the client
		 */
		void stop()
		{
			if (m_client_impl_ptr)
			{
				m_client_impl_ptr->stop();
			}
		}

		bool is_start()
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->is_start() : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(std::shared_ptr<buffer<uint8_t>> buf_ptr)
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->send(std::move(buf_ptr)) : false);
		}

		/**
		 * @function : send data
		 */
		virtual bool send(const uint8_t * buf, std::size_t len)
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->send(buf, len) : false);
		}

		/**
		 * @function : send data, inner use std::strlen(buf) to calc the buf len.
		 */
		virtual bool send(const char * buf)
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->send(buf) : false);
		}

	public:
		/**
		 * @function : set the data parser under pack model,you must call set_pack_parser before call server::start()
		 * the pack parser function like this :
		 * std::size_t pack_parser(std::shared_ptr<buffer<uint8_t>> & buf_ptr)
		 */
		template<typename _parser>
		client & set_pack_parser(const _parser & parser)
		{
			try
			{
				if/**/ (m_url_parser_ptr->get_protocol() == "tcp")
				{
					std::dynamic_pointer_cast<tcp_pack_client>(m_client_impl_ptr)->set_pack_parser(parser);
				}
				else if (m_url_parser_ptr->get_protocol() == "tcps")
				{
#if defined(ASIO2_USE_SSL)
					std::dynamic_pointer_cast<tcps_pack_client>(m_client_impl_ptr)->set_pack_parser(parser);
#else
					throw std::runtime_error("you must #define ASIO2_USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
				}
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

#if defined(ASIO2_USE_SSL)
	public:
		client & set_certificate(std::string buffer)
		{
			try
			{
				std::static_pointer_cast<tcps_client>(m_client_impl_ptr)->get_ssl_context()->add_certificate_authority(
					boost::asio::const_buffer(buffer.data(), buffer.size()));
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
				assert(false);
			}
			return (*this);
		}
		client & set_certificate_file(std::string file)
		{
			try
			{
				std::static_pointer_cast<tcps_client>(m_client_impl_ptr)->get_ssl_context()->load_verify_file(file);
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value());
				assert(false);
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
		client & bind_listener(std::shared_ptr<client_listener> listener_sptr)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_sptr);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 * must ensure the listener_sptr is valid before server has stoped 
		 */
		client & bind_listener(std::shared_ptr<http_client_listener> listener_sptr)
		{
			try
			{
				std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_sptr);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 * must ensure the listener_rptr is valid before server has stoped 
		 */
		client & bind_listener(client_listener                * listener_rptr)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_rptr);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 * must ensure the listener_rptr is valid before server has stoped 
		 */
		client & bind_listener(http_client_listener                * listener_rptr)
		{
			try
			{
				std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_listener(listener_rptr);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client send data finished
		 * @param    : listener - a callback function
		 * the callback function like this : 
		 * void on_send(asio2::buffer_ptr & buf_ptr, int error)
		 * or a lumbda function like this : 
		 * [&](asio2::buffer_ptr & buf_ptr, int error){}
		 */
		client & bind_send(std::function<client_listener_mgr::send_callback> listener)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_send(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client send data finished
		 * @param    : listener - a callback function
		 * the callback function like this : 
		 * void on_send(asio2::buffer_ptr & buf_ptr, int error)
		 * or a lumbda function like this : 
		 * [&](asio2::buffer_ptr & buf_ptr, int error){}
		 */
		client & bind_send(std::function<http_client_listener_mgr::send_callback> listener)
		{
			try
			{
				std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_send(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client recv data from remote endpoint
		 * @param    : listener - a callback function like this:
		 * void on_recv(asio2::buffer_ptr & buf_ptr)
		 * or a lambda function like this :
		 * [&](asio2::buffer_ptr & buf_ptr){}
		 */
		client & bind_recv(std::function<client_listener_mgr::recv_callback> listener)
		{
			try
			{
				std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_recv(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client recv data from remote endpoint
		 * @param    : listener - a callback function like this:
		 * void on_recv(asio2::buffer_ptr & buf_ptr)
		 * or a lambda function like this :
		 * [&](asio2::buffer_ptr & buf_ptr){}
		 */
		client & bind_recv(std::function<http_client_listener_mgr::recv_callback> listener)
		{
			try
			{
				std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_recv(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client is closed
		 * @param    : listener - a callback function like this:
		 * void on_close(int error)
		 */
		client & bind_close(std::function<client_listener_mgr::clos_callback> listener)
		{
			try
			{
				if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
				else if (m_url_parser_ptr->get_protocol() == "tcps")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
				else if (m_url_parser_ptr->get_protocol() == "udp")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
				else if (m_url_parser_ptr->get_protocol() == "http")
					std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
				else if (m_url_parser_ptr->get_protocol() == "https")
					std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
				else if (m_url_parser_ptr->get_protocol() == "rpc")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_close(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

		/**
		 * @function : bind listener - the client connect to the server finished,may be connected failed.
		 * @param    : listener - a callback function like this:
		 * void on_connect(int error)
		 */
		client & bind_connect(std::function<client_listener_mgr::conn_callback> listener)
		{
			try
			{
				if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
				else if (m_url_parser_ptr->get_protocol() == "tcps")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
				else if (m_url_parser_ptr->get_protocol() == "udp")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
				else if (m_url_parser_ptr->get_protocol() == "http")
					std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
				else if (m_url_parser_ptr->get_protocol() == "https")
					std::dynamic_pointer_cast<http_client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
				else if (m_url_parser_ptr->get_protocol() == "rpc")
					std::dynamic_pointer_cast<client_listener_mgr>(m_listener_mgr_ptr)->bind_connect(listener);
			}
			catch (std::exception &) { assert(false); }
			return (*this);
		}

	public:
		/**
		 * @function : get the local address
		 * note : if you closed the session manual,when the _fire_close is called,this function
		 *        can't get the correctly ip address,it will return a empty string
		 */
		inline std::string get_local_address()
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->get_local_address() : std::string());
		}

		/**
		 * @function : get the local port
		 * note : if you closed the session manual,when the _fire_close is called,this function
		 *        can't get the correctly port num,it will return zero
		 */
		inline unsigned short get_local_port()
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->get_local_port() : static_cast<unsigned short>(0));
		}

		/**
		 * @function : get the remote address
		 * note : if you closed the session manual,when the _fire_close is called,this function
		 *        can't get the correctly ip address,it will return a empty string
		 */
		inline std::string get_remote_address()
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->get_remote_address() : std::string());
		}

		/**
		 * @function : get the remote port
		 * note : if you closed the session manual,when the _fire_close is called,this function
		 *        can't get the correctly port num,it will return zero
		 */
		inline unsigned short get_remote_port()
		{
			return (m_client_impl_ptr ? m_client_impl_ptr->get_remote_port() : static_cast<unsigned short>(0));
		}

	protected:
		void _init(std::string & url
#if defined(ASIO2_USE_SSL)
			, boost::asio::ssl::context::method  method
			, boost::asio::ssl::context::options options
#endif
		)
		{
			m_url_parser_ptr = std::make_shared<url_parser>(url);

			if /**/ (m_url_parser_ptr->get_protocol() == "tcp")
				m_listener_mgr_ptr = std::make_shared<client_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "tcps")
				m_listener_mgr_ptr = std::make_shared<client_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "udp")
				m_listener_mgr_ptr = std::make_shared<client_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "http")
				m_listener_mgr_ptr = std::make_shared<http_client_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "https")
				m_listener_mgr_ptr = std::make_shared<http_client_listener_mgr>();
			else if (m_url_parser_ptr->get_protocol() == "rpc")
				m_listener_mgr_ptr = std::make_shared<client_listener_mgr>();

			if/**/ (m_url_parser_ptr->get_protocol() == "tcp")
			{
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					m_client_impl_ptr = std::make_shared<tcp_auto_client>(m_url_parser_ptr, m_listener_mgr_ptr);
				else if (m_url_parser_ptr->get_model() == "pack")
					m_client_impl_ptr = std::make_shared<tcp_pack_client>(m_url_parser_ptr, m_listener_mgr_ptr);
				else
					m_client_impl_ptr = std::make_shared<tcp_client>     (m_url_parser_ptr, m_listener_mgr_ptr);
			}
			else if (m_url_parser_ptr->get_protocol() == "tcps")
			{
#if defined(ASIO2_USE_SSL)
				if /**/ (m_url_parser_ptr->get_model() == "auto")
					m_client_impl_ptr = std::make_shared<tcps_auto_client>(m_url_parser_ptr, m_listener_mgr_ptr, method, options);
				else if (m_url_parser_ptr->get_model() == "pack")
					m_client_impl_ptr = std::make_shared<tcps_pack_client>(m_url_parser_ptr, m_listener_mgr_ptr, method, options);
				else
					m_client_impl_ptr = std::make_shared<tcps_client>     (m_url_parser_ptr, m_listener_mgr_ptr, method, options);
#else
				throw std::runtime_error("you must #define ASIO2_USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
			}
			else if (m_url_parser_ptr->get_protocol() == "udp")
				m_client_impl_ptr = std::make_shared<udp_client>(m_url_parser_ptr, m_listener_mgr_ptr);
			else if (m_url_parser_ptr->get_protocol() == "http")
				m_client_impl_ptr = std::make_shared<http_client>(m_url_parser_ptr, m_listener_mgr_ptr);
			else if (m_url_parser_ptr->get_protocol() == "https")
#if defined(ASIO2_USE_SSL)
				m_client_impl_ptr = std::make_shared<http_client>(m_url_parser_ptr, m_listener_mgr_ptr);
#else
				throw std::runtime_error("you must #define ASIO2_USE_SSL macro before #include <asio2/asio2.hpp>");
#endif
			else if (m_url_parser_ptr->get_protocol() == "rpc")
				m_client_impl_ptr = std::make_shared<udp_client>(m_url_parser_ptr, m_listener_mgr_ptr);
		}

	protected:
		std::shared_ptr<url_parser  >        m_url_parser_ptr;
		std::shared_ptr<listener_mgr>        m_listener_mgr_ptr;
		std::shared_ptr<client_impl >        m_client_impl_ptr;

	};
}

#endif // !__ASIO2_CLIENT_HPP__
