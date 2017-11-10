/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_LISTENER_HPP__
#define __ASIO2_LISTENER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>

#include <boost/asio.hpp>

#include <asio2/util/buffer.hpp>
#include <asio2/http/http_request.hpp>
#include <asio2/http/http_response.hpp>

namespace asio2
{

	class session_impl;

	/**
	 * the base listener interface 
	 */
	class listener_t
	{
	public:

		/**
		 * @construct
		 */
		listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~listener_t()
		{
		}

	};

	/**
	 * the server listener interface 
	 */
	template <typename _session, typename _format>
	class server_listener_t : public listener_t
	{
	public:

		/**
		 * @construct
		 */
		server_listener_t() : listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~server_listener_t()
		{
		}

		/**
		 * @function : handle the send data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr - the send data session shared_ptr
		 *             data_ptr    - the send data buffer shared_ptr
		 *             len         - the send data buffer len
		 */
		virtual void on_send(std::shared_ptr<_session> session_ptr, std::shared_ptr<buffer<_format>> data_ptr, int error) {};

		/**
		 * @function : handle the recv data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr - the recv data session shared_ptr
		 *             data_ptr    - the recv data buffer shared_ptr
		 *             len         - the recv data buffer len
		 */
		virtual void on_recv(std::shared_ptr<_session> session_ptr, std::shared_ptr<buffer<_format>> data_ptr) {};

		/**
		 * @function : handle the session closed notify
		 * @param    : session_ptr - the closing session shared_ptr
		 *             error - error code
		 * when on_close is called,the session's socket is closed,so if you call get_local_address or get_local_port and so on in 
		 * on_close function,it will not get correct result.
		 */
		virtual void on_close(std::shared_ptr<_session> session_ptr, int error) {};

		/**
		 * @function : handle the listen notify,you can't call server_ptr->stop(); directly to close the server.
		 */
		virtual void on_listen() {};

		/**
		 * @function : handle the new connected session notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr  - the new connected session shared_ptr
		 */
		virtual void on_accept(std::shared_ptr<_session> session_ptr) {};

		/**
		 * @function : handle the server's acceptor closed notify,when the acceptor is closed,we will get this notify
		 */
		virtual void on_shutdown(int error) {};

	};

	/**
	 * the client listener interface 
	 */
	template <typename _format>
	class client_listener_t : public listener_t
	{
	public:

		/**
		 * @construct
		 */
		client_listener_t() : listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~client_listener_t()
		{
		}

		/**
		 * @function : handle the send data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : data_ptr    - the send data buffer shared_ptr
		 *             len         - the send data buffer len
		 */
		virtual void on_send(std::shared_ptr<buffer<_format>> data_ptr, int error) {};

		/**
		 * @function : handle the recv data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : data_ptr    - the recv data buffer shared_ptr
		 *             len         - the recv data buffer len
		 */
		virtual void on_recv(std::shared_ptr<buffer<_format>> data_ptr) {};

		/**
		 * @function : handle the session closed notify
		 * @param    : error - error code
		 * when on_close is called,the session's socket is closed,so if you call get_local_address or get_local_port and so on in 
		 * on_close function,it will not get correct result.
		 */
		virtual void on_close(int error) {};

		/**
		 * @function : handle the connect to server successed notify,you can't call client_ptr->stop(); directly to close the client.
		 * @param    : error - error code
		 */
		virtual void on_connect(int error) {};

	};

	/**
	 * the udp sender listener interface 
	 */
	template <typename _format>
	class sender_listener_t : public listener_t
	{
	public:

		/**
		 * @construct
		 */
		sender_listener_t() : listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~sender_listener_t()
		{
		}

		/**
		 * @function : handle the send data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : data_ptr    - the send data buffer shared_ptr
		 *             len         - the send data buffer len
		 */
		virtual void on_send(std::string ip, unsigned short port, std::shared_ptr<buffer<_format>> data_ptr, int error) {};

		/**
		 * @function : handle the recv data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : data_ptr    - the recv data buffer shared_ptr
		 *             len         - the recv data buffer len
		 */
		virtual void on_recv(std::string ip, unsigned short port, std::shared_ptr<buffer<_format>> data_ptr) {};

		/**
		 * @function : handle the session closed notify
		 * @param    : error - error code
		 * when on_close is called,the session's socket is closed,so if you call get_local_address or get_local_port and so on in 
		 * on_close function,it will not get correct result.
		 */
		virtual void on_close(int error) {};

	};

	/**
	 * the tcp server listener interface 
	 */
	template <typename _session, typename _format>
	class tcp_server_listener_t : public server_listener_t<_session, _format>
	{
	public:

		/**
		 * @construct
		 */
		tcp_server_listener_t() : server_listener_t<_session, _format>()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_server_listener_t()
		{
		}

	};

	/**
	 * the udp server listener interface 
	 */
	template <typename _session, typename _format>
	class udp_server_listener_t : public server_listener_t<_session, _format>
	{
	public:

		/**
		 * @construct
		 */
		udp_server_listener_t() : server_listener_t<_session, _format>()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~udp_server_listener_t()
		{
		}

	};

	/**
	 * the http server listener interface 
	 */
	template <typename _session, typename _format>
	class http_server_listener_t : public listener_t
	{
	public:

		/**
		 * @construct
		 */
		http_server_listener_t() : listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_server_listener_t()
		{
		}

		/**
		 * @function : handle the send data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr  - the send data session shared_ptr
		 *             response_ptr - the send data buffer shared_ptr
		 *             len          - the send data buffer len
		 */
		virtual void on_send(std::shared_ptr<_session> session_ptr, std::shared_ptr<http_response> response_ptr, int error) {};

		/**
		 * @function : handle the recv data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr - the recv data session shared_ptr
		 *             request_ptr - the recv data buffer shared_ptr
		 *             len         - the recv data buffer len
		 */
		virtual void on_recv(std::shared_ptr<_session> session_ptr, std::shared_ptr<http_request> request_ptr) {};

		/**
		 * @function : handle the session closed notify
		 * @param    : session_ptr - the closing session shared_ptr
		 *             error - error code
		 * when on_close is called,the session's socket is closed,so if you call get_local_address or get_local_port and so on in 
		 * on_close function,it will not get correct result.
		 */
		virtual void on_close(std::shared_ptr<_session> session_ptr, int error) {};

		/**
		 * @function : handle the listen notify,you can't call server_ptr->stop(); directly to close the server.
		 */
		virtual void on_listen() {};

		/**
		 * @function : handle the new connected session notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : session_ptr  - the new connected session shared_ptr
		 */
		virtual void on_accept(std::shared_ptr<_session> session_ptr) {};

		/**
		 * @function : handle the server's acceptor closed notify,when the acceptor is closed,we will get this notify
		 */
		virtual void on_shutdown(int error) {};

	};

	/**
	 * the tcp client listener interface 
	 */
	template <typename _format>
	class tcp_client_listener_t : public client_listener_t<_format>
	{
	public:

		/**
		 * @construct
		 */
		tcp_client_listener_t() : client_listener_t<_format>()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~tcp_client_listener_t()
		{
		}

	};

	/**
	 * the udp client listener interface 
	 */
	template <typename _format>
	class udp_client_listener_t : public client_listener_t<_format>
	{
	public:

		/**
		 * @construct
		 */
		udp_client_listener_t() : client_listener_t<_format>()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~udp_client_listener_t()
		{
		}

	};

	/**
	 * the client listener interface 
	 */
	template <typename _format>
	class http_client_listener_t : public listener_t
	{
	public:

		/**
		 * @construct
		 */
		http_client_listener_t() : listener_t()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_client_listener_t()
		{
		}

		/**
		 * @function : handle the send data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : request_ptr - the send data buffer shared_ptr
		 *             len         - the send data buffer len
		 */
		virtual void on_send(std::shared_ptr<http_request> request_ptr, int error) {};

		/**
		 * @function : handle the recv data notify,you can call session_ptr->stop(); directly to close the session.
		 * @param    : response_ptr - the recv data buffer shared_ptr
		 *             len          - the recv data buffer len
		 */
		virtual void on_recv(std::shared_ptr<http_response> response_ptr) {};

		/**
		 * @function : handle the session closed notify
		 * @param    : error - error code
		 * when on_close is called,the session's socket is closed,so if you call get_local_address or get_local_port and so on in 
		 * on_close function,it will not get correct result.
		 */
		virtual void on_close(int error) {};

		/**
		 * @function : handle the connect to server successed notify,you can't call client_ptr->stop(); directly to close the client.
		 * @param    : error - error code
		 */
		virtual void on_connect(int error) {};

	};

	using server_listener      = server_listener_t<session_impl, uint8_t>;
	using tcp_server_listener  = tcp_server_listener_t<session_impl, uint8_t>;
	using udp_server_listener  = udp_server_listener_t<session_impl, uint8_t>;
	using http_server_listener = http_server_listener_t<session_impl, uint8_t>;

	using client_listener      = client_listener_t<uint8_t>;
	using tcp_client_listener  = tcp_client_listener_t<uint8_t>;
	using udp_client_listener  = udp_client_listener_t<uint8_t>;
	using http_client_listener = http_client_listener_t<uint8_t>;

	using sender_listener      = sender_listener_t<uint8_t>;

}

#endif // !__ASIO2_LISTENER_HPP__
