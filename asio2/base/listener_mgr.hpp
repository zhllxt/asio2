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

#include <asio2/util/buffer.hpp>

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

		using send_callback = void(std::shared_ptr<session_impl> & session_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr, int error);
		using recv_callback = void(std::shared_ptr<session_impl> & session_ptr, std::shared_ptr<buffer<uint8_t>> & buf_ptr);
		using lisn_callback = void();
		using acpt_callback = void(std::shared_ptr<session_impl> & session_ptr);
		using clos_callback = void(std::shared_ptr<session_impl> & session_ptr, int error);
		using shut_callback = void(int error);

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
		server_listener_mgr & bind_listener(const std::shared_ptr<_listener_t> & listener_sptr)
		{
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		server_listener_mgr & bind_listener(_listener_t                  * listener_rptr)
		{
			std::shared_ptr<_listener_t> listener_sptr(listener_rptr, [](_listener_t *) {});
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		server_listener_mgr & bind_send(const _listener & listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_recv(const _listener & listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_listen(const _listener & listener)
		{
			m_lisn_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_accept(const _listener & listener)
		{
			m_acpt_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_close(const _listener & listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		server_listener_mgr & bind_shutdown(const _listener & listener)
		{
			m_shut_listener = listener;
			return (*this);
		}

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_listen(Args&&... args)
		{
			if (m_lisn_listener)
				m_lisn_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_listen(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_accept(Args&&... args)
		{
			if (m_acpt_listener)
				m_acpt_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_accept(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_shutdown(Args&&... args)
		{
			if (m_shut_listener)
				m_shut_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_shutdown(std::forward<Args>(args)...);
		}

	protected:

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<lisn_callback> m_lisn_listener = nullptr;
		std::function<acpt_callback> m_acpt_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<shut_callback> m_shut_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_ptr = nullptr;
	};


	/**
	 * the client listener manager interface
	 */
	class client_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = client_listener_t<uint8_t>;

		using send_callback = void(std::shared_ptr<buffer<uint8_t>> & buf_ptr, int error);
		using recv_callback = void(std::shared_ptr<buffer<uint8_t>> & buf_ptr);
		using clos_callback = void(int error);
		using conn_callback = void(int error);

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
		client_listener_mgr & bind_listener(const std::shared_ptr<_listener_t> & listener_sptr)
		{
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		client_listener_mgr & bind_listener(_listener_t                  * listener_rptr)
		{
			std::shared_ptr<_listener_t> listener_sptr(listener_rptr, [](_listener_t *) {});
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		client_listener_mgr & bind_send(const _listener & listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_recv(const _listener & listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_close(const _listener & listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		client_listener_mgr & bind_connect(const _listener & listener)
		{
			m_conn_listener = listener;
			return (*this);
		}

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_connect(Args&&... args)
		{
			if (m_conn_listener)
				m_conn_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_connect(std::forward<Args>(args)...);
		}

	protected:

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<conn_callback> m_conn_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_ptr = nullptr;
	};


	/**
	 * the sender listener manager interface
	 */
	class sender_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = sender_listener_t<uint8_t>;

		using send_callback = void(std::string & ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> & buf_ptr, int error);
		using recv_callback = void(std::string & ip, unsigned short port, std::shared_ptr<buffer<uint8_t>> & buf_ptr);
		using clos_callback = void(int error);

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
		sender_listener_mgr & bind_listener(const std::shared_ptr<_listener_t> & listener_sptr)
		{
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		sender_listener_mgr & bind_listener(_listener_t                  * listener_rptr)
		{
			std::shared_ptr<_listener_t> listener_sptr(listener_rptr, [](_listener_t *) {});
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		sender_listener_mgr & bind_send(const _listener & listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		sender_listener_mgr & bind_recv(const _listener & listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		sender_listener_mgr & bind_close(const _listener & listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_close(std::forward<Args>(args)...);
		}

	protected:

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_ptr = nullptr;
	};



	/**
	 * the http server listener manager interface
	 */
	class http_server_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = http_server_listener_t<session_impl, uint8_t>;

		using send_callback = void(std::shared_ptr<session_impl> & session_ptr, std::shared_ptr<http_response> & response_ptr, int error);
		using recv_callback = void(std::shared_ptr<session_impl> & session_ptr, std::shared_ptr<http_request > & request_ptr);
		using lisn_callback = void();
		using acpt_callback = void(std::shared_ptr<session_impl> & session_ptr);
		using clos_callback = void(std::shared_ptr<session_impl> & session_ptr, int error);
		using shut_callback = void(int error);

		/**
		 * @construct
		 */
		http_server_listener_mgr() : listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_server_listener_mgr()
		{
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 */
		http_server_listener_mgr & bind_listener(const std::shared_ptr<_listener_t> & listener_sptr)
		{
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		http_server_listener_mgr & bind_listener(_listener_t                  * listener_rptr)
		{
			std::shared_ptr<_listener_t> listener_sptr(listener_rptr, [](_listener_t *) {});
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		http_server_listener_mgr & bind_send(const _listener & listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_server_listener_mgr & bind_recv(const _listener & listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_server_listener_mgr & bind_listen(const _listener & listener)
		{
			m_lisn_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_server_listener_mgr & bind_accept(const _listener & listener)
		{
			m_acpt_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_server_listener_mgr & bind_close(const _listener & listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_server_listener_mgr & bind_shutdown(const _listener & listener)
		{
			m_shut_listener = listener;
			return (*this);
		}

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_listen(Args&&... args)
		{
			if (m_lisn_listener)
				m_lisn_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_listen(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_accept(Args&&... args)
		{
			if (m_acpt_listener)
				m_acpt_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_accept(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_shutdown(Args&&... args)
		{
			if (m_shut_listener)
				m_shut_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_shutdown(std::forward<Args>(args)...);
		}

	protected:

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<lisn_callback> m_lisn_listener = nullptr;
		std::function<acpt_callback> m_acpt_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<shut_callback> m_shut_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_ptr = nullptr;
	};


	/**
	 * the http client listener manager interface
	 */
	class http_client_listener_mgr : public listener_mgr
	{
	public:

		using _listener_t = http_client_listener_t<uint8_t>;

		using send_callback = void(std::shared_ptr<http_request > & request_ptr, int error);
		using recv_callback = void(std::shared_ptr<http_response> & response_ptr);
		using clos_callback = void(int error);
		using conn_callback = void(int error);

		/**
		 * @construct
		 */
		http_client_listener_mgr() : listener_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_client_listener_mgr()
		{
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_sptr - a listener object shared_ptr
		 */
		http_client_listener_mgr & bind_listener(const std::shared_ptr<_listener_t> & listener_sptr)
		{
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - 
		 * @param    : listener_rptr - a listener object raw pointer
		 */
		http_client_listener_mgr & bind_listener(_listener_t                  * listener_rptr)
		{
			std::shared_ptr<_listener_t> listener_sptr(listener_rptr, [](_listener_t *) {});
			m_listener_ptr = std::move(listener_sptr);
			return (*this);
		}

		/**
		 * @function : bind listener - "send","recv","listen","accept","close","shutdown"
		 * @param    : listener - a callback function
		 */
		template<typename _listener>
		http_client_listener_mgr & bind_send(const _listener & listener)
		{
			m_send_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_client_listener_mgr & bind_recv(const _listener & listener)
		{
			m_recv_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_client_listener_mgr & bind_close(const _listener & listener)
		{
			m_clos_listener = listener;
			return (*this);
		}

		template<typename _listener>
		http_client_listener_mgr & bind_connect(const _listener & listener)
		{
			m_conn_listener = listener;
			return (*this);
		}

		/**
		 * @function : call the listener callback function
		 */
		template<typename... Args>
		inline void notify_send(Args&&... args)
		{
			if (m_send_listener)
				m_send_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_send(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_recv(Args&&... args)
		{
			if (m_recv_listener)
				m_recv_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_recv(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_close(Args&&... args)
		{
			if (m_clos_listener)
				m_clos_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_close(std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void notify_connect(Args&&... args)
		{
			if (m_conn_listener)
				m_conn_listener(std::forward<Args>(args)...);
			else if (m_listener_ptr)
				m_listener_ptr->on_connect(std::forward<Args>(args)...);
		}

	protected:

		std::function<send_callback> m_send_listener = nullptr;
		std::function<recv_callback> m_recv_listener = nullptr;
		std::function<clos_callback> m_clos_listener = nullptr;
		std::function<conn_callback> m_conn_listener = nullptr;

	protected:

		std::shared_ptr<_listener_t> m_listener_ptr = nullptr;
	};

}

#endif // !__ASIO2_LISTENER_MGR_HPP__
