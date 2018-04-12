/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCPS_SERVER_IMPL_HPP__
#define __ASIO2_TCPS_SERVER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_server_impl.hpp>

#include <asio2/tcp/tcps_acceptor_impl.hpp>

namespace asio2
{

	template<class _acceptor_impl_t>
	class tcps_server_impl : public tcp_server_impl<_acceptor_impl_t>
	{
	public:
		typedef _acceptor_impl_t acceptor_impl_t;
		typedef typename _acceptor_impl_t::session_impl_t session_impl_t;

		/**
		 * @construct
		 */
		tcps_server_impl(
			std::shared_ptr<url_parser>        url_parser_ptr,
			std::shared_ptr<listener_mgr>      listener_mgr_ptr,
			asio::ssl::context::method  method,
			asio::ssl::context::options options
		)
			: tcp_server_impl<_acceptor_impl_t>(url_parser_ptr, nullptr)
		{
			this->m_listener_mgr_ptr = listener_mgr_ptr;
			try
			{
				this->m_ssl_context_ptr = std::make_shared<asio::ssl::context>(method);
				this->m_ssl_context_ptr->set_options(options);

				this->m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(url_parser_ptr, listener_mgr_ptr, this->m_io_context_pool_ptr, this->m_ssl_context_ptr);
			}
			catch (asio::system_error & e)
			{
				set_last_error(e.code().value());
				ASIO2_DUMP_EXCEPTION_LOG_IMPL;
				m_ssl_context_ptr.reset();
			}
		}

		/**
		 * @destruct
		 */
		virtual ~tcps_server_impl()
		{
		}

		virtual bool start() override
		{
			return ((this->m_ssl_context_ptr && this->m_acceptor_impl_ptr) ? tcp_server_impl<_acceptor_impl_t>::start() : false);
		}

		virtual void stop() override
		{
			tcp_server_impl<_acceptor_impl_t>::stop();

			//CONF_modules_unload(1);        //for conf
			//EVP_cleanup();                 //For EVP
			//ENGINE_cleanup();              //for engine
			//CRYPTO_cleanup_all_ex_data();  //generic 
			//ERR_remove_state(0);           //for ERR
			//ERR_free_strings();            //for ERR

			//// thread-local cleanup
			//ERR_remove_state(0);

			//// thread-safe cleanup
			//ENGINE_cleanup();
			//CONF_modules_unload(1);

			//// global application exit cleanup (after all SSL activity is shutdown)
			//ERR_free_strings();
			//EVP_cleanup();
			//CRYPTO_cleanup_all_ex_data();
		}

		inline std::shared_ptr<asio::ssl::context> get_ssl_context() { return this->m_ssl_context_ptr; }

		tcps_server_impl & set_password(std::string password)
		{
			this->m_ssl_context_ptr->set_password_callback([password]
			(std::size_t max_length, asio::ssl::context_base::password_purpose purpose) -> std::string
			{
				return password;
			});
			return (*this);
		}

	protected:
		/// ssl context 
		std::shared_ptr<asio::ssl::context> m_ssl_context_ptr;

	};

}

#endif // !__ASIO2_TCPS_SERVER_IMPL_HPP__
