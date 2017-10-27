/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * https://stackoverflow.com/questions/7624017/c0x-storing-any-type-of-stdfunction-in-a-stdmap
 */

#ifndef __ASIO2_LISTENER_MGR_HPP__
#define __ASIO2_LISTENER_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/pool.hpp>

#include <asio2/base/listener.hpp>

namespace asio2
{
	class session_impl;

	/**
	 * the base listener manager interface
	 */
	class listener_mgr
	{
	public:

		/**
		 * @construct
		 */
		listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~listener_mgr()
		{
		}

	};


	/**
	 * the server listener manager interface
	 */
	class server_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = server_listener_t<session_impl, uint8_t>;

		/**
		 * @construct
		 */
		server_listener_mgr() : listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~server_listener_mgr()
		{
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 */
		listener_mgr & bind_listener(std::shared_ptr<_listener_t> listener_sptr)
		{
			m_listener_sptr = listener_sptr;
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		listener_mgr & bind_listener(_listener_t                * listener_rptr)
		{
			m_listener_rptr = listener_rptr;
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		server_listener_mgr & bind_send(_listener listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_recv(_listener listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_listen(_listener listener)
		{
			m_lisn_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_accept(_listener listener)
		{
			m_acpt_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_close(_listener listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_shutdown(_listener listener)
		{
			m_shut_listener = listener;
			return (*this);
		}

		inline bool is_send_listener_exist()     { return (m_send_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_recv_listener_exist()     { return (m_recv_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_listen_listener_exist()   { return (m_lisn_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_accept_listener_exist()   { return (m_acpt_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_close_listener_exist()    { return (m_clos_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_shutdown_listener_exist() { return (m_shut_listener || m_listener_sptr || m_listener_rptr); }

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_send(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_recv(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_listen(Args&&... args)
		{
			if (m_lisn_listener)
				m_lisn_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_listen(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_listen(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_accept(Args&&... args)
		{
			if (m_acpt_listener)
				m_acpt_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_accept(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_accept(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_close(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_shutdown(Args&&... args)
		{
			if (m_shut_listener)
				m_shut_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_shutdown(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_shutdown(std::forward<Args>(args)...);
		}

	protected:

		using send_callback = void(std::shared_ptr<session_impl> session_ptr, std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error);
		using recv_callback = void(std::shared_ptr<session_impl> session_ptr, std::shared_ptr<uint8_t> data_ptr, std::size_t len);
		using lisn_callback = void();
		using acpt_callback = void(std::shared_ptr<session_impl> session_ptr);
		using clos_callback = void(std::shared_ptr<session_impl> session_ptr, int error);
		using shut_callback = void(int error);

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<lisn_callback> m_lisn_listener = nullptr;
		std::function<acpt_callback> m_acpt_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<shut_callback> m_shut_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_sptr = nullptr;
		_listener_t                * m_listener_rptr = nullptr;
	};


	/**
	 * the client listener manager interface
	 */
	class client_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = client_listener_t<uint8_t>;

		/**
		 * @construct
		 */
		client_listener_mgr() : listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~client_listener_mgr()
		{
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 */
		listener_mgr & bind_listener(std::shared_ptr<_listener_t> listener_sptr)
		{
			m_listener_sptr = listener_sptr;
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		listener_mgr & bind_listener(_listener_t                * listener_rptr)
		{
			m_listener_rptr = listener_rptr;
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		client_listener_mgr & bind_send(_listener listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_recv(_listener listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_close(_listener listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_connect(_listener listener)
		{
			m_conn_listener = listener;
			return (*this);
		}

		inline bool is_send_listener_exist()     { return (m_send_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_recv_listener_exist()     { return (m_recv_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_close_listener_exist()    { return (m_clos_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_connect_listener_exist()  { return (m_conn_listener || m_listener_sptr || m_listener_rptr); }

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_send(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_recv(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_close(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_connect(Args&&... args)
		{
			if (m_conn_listener)
				m_conn_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_connect(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_connect(std::forward<Args>(args)...);
		}

	protected:

		using send_callback = void(std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error);
		using recv_callback = void(std::shared_ptr<uint8_t> data_ptr, std::size_t len);
		using clos_callback = void(int error);
		using conn_callback = void(int error);

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<conn_callback> m_conn_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_sptr = nullptr;
		_listener_t                * m_listener_rptr = nullptr;
	};


	/**
	 * the sender listener manager interface
	 */
	class sender_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = sender_listener_t<uint8_t>;

		/**
		 * @construct
		 */
		sender_listener_mgr() : listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~sender_listener_mgr()
		{
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 */
		listener_mgr & bind_listener(std::shared_ptr<_listener_t> listener_sptr)
		{
			m_listener_sptr = listener_sptr;
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		listener_mgr & bind_listener(_listener_t                * listener_rptr)
		{
			m_listener_rptr = listener_rptr;
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		sender_listener_mgr & bind_send(_listener listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		sender_listener_mgr & bind_recv(_listener listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		sender_listener_mgr & bind_close(_listener listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		inline bool is_send_listener_exist()     { return (m_send_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_recv_listener_exist()     { return (m_recv_listener || m_listener_sptr || m_listener_rptr); }
		inline bool is_close_listener_exist()    { return (m_clos_listener || m_listener_sptr || m_listener_rptr); }

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_send(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_recv(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_sptr)
				m_listener_sptr->on_close(std::forward<Args>(args)...);
			else if (m_listener_rptr)
				m_listener_rptr->on_close(std::forward<Args>(args)...);
		}

	protected:

		using send_callback = void(std::string ip, unsigned short port, std::shared_ptr<uint8_t> data_ptr, std::size_t len, int error);
		using recv_callback = void(std::string ip, unsigned short port, std::shared_ptr<uint8_t> data_ptr, std::size_t len);
		using clos_callback = void(int error);

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_sptr = nullptr;
		_listener_t                * m_listener_rptr = nullptr;
	};


}

#endif // !__ASIO2_LISTENER_MGR_HPP__
