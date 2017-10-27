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

#include <boost/asio/ssl.hpp>

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
		typedef typename _acceptor_impl_t::pool_t pool_t;

		/**
		 * @construct
		 */
		tcps_server_impl(
			std::shared_ptr<listener_mgr> listener_mgr_ptr,
			std::shared_ptr<url_parser> url_parser_ptr
		)
			: tcp_server_impl<_acceptor_impl_t>(listener_mgr_ptr, url_parser_ptr)
		{
			try
			{
				m_context_ptr = std::make_shared<boost::asio::ssl::context>(boost::asio::ssl::context::sslv23);
				m_context_ptr->set_options(
					boost::asio::ssl::context::default_workarounds |
					boost::asio::ssl::context::no_sslv2 |
					boost::asio::ssl::context::single_dh_use
				);
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
				PRINT_EXCEPTION;
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
			return tcp_server_impl<_acceptor_impl_t>::start();
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

			// thread-local cleanup
			ERR_remove_state(0);

			// thread-safe cleanup
			ENGINE_cleanup();
			CONF_modules_unload(1);

			// global application exit cleanup (after all SSL activity is shutdown)
			ERR_free_strings();
			EVP_cleanup();
			CRYPTO_cleanup_all_ex_data();
		}

		inline std::shared_ptr<boost::asio::ssl::context> get_context_ptr() { return m_context_ptr; }

		tcps_server_impl & set_password(std::string password)
		{
			m_context_ptr->set_password_callback([password]
			(std::size_t max_length, boost::asio::ssl::context_base::password_purpose purpose) -> std::string
			{
				return password;
			});
			return (*this);
		}

	protected:

		virtual bool _start_listen() override
		{
			try
			{
				this->m_acceptor_impl_ptr = std::make_shared<_acceptor_impl_t>(
					this->m_io_service_pool_evt_ptr,
					this->m_io_service_pool_msg_ptr,
					this->m_listener_mgr_ptr,
					this->m_url_parser_ptr,
					this->m_recv_buf_pool_ptr
					);

				std::dynamic_pointer_cast<_acceptor_impl_t>(this->m_acceptor_impl_ptr)->set_context(m_context_ptr);

				return this->m_acceptor_impl_ptr->start();
			}
			catch (boost::system::system_error & e)
			{
				set_last_error(e.code().value(), e.what());
			}

			return false;
		}

	protected:
		
		/// ssl context 
		std::shared_ptr<boost::asio::ssl::context> m_context_ptr;

	};

}

#endif // !__ASIO2_TCPS_SERVER_IMPL_HPP__
