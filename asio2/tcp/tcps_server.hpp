/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#if defined(ASIO2_USE_SSL)

#ifndef __ASIO2_TCPS_SERVER_HPP__
#define __ASIO2_TCPS_SERVER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/tcp/tcp_server.hpp>
#include <asio2/tcp/tcps_session.hpp>

namespace asio2::detail
{
	template<class derived_t, class session_t>
	class tcps_server_impl_t
		: public asio::ssl::context
		, public tcp_server_impl_t<derived_t, session_t>
	{
		template <class, bool>  friend class user_timer_cp;
		template <class, class> friend class server_impl_t;
		template <class, class> friend class tcp_server_impl_t;

	public:
		using self = tcps_server_impl_t<derived_t, session_t>;
		using super = tcp_server_impl_t<derived_t, session_t>;
		using session_type = session_t;

		/**
		 * @constructor
		 */
		explicit tcps_server_impl_t(
			asio::ssl::context::method method = asio::ssl::context::sslv23,
			std::size_t init_buffer_size = tcp_frame_size,
			std::size_t max_buffer_size = (std::numeric_limits<std::size_t>::max)(),
			std::size_t concurrency = std::thread::hardware_concurrency() * 2
		)
			: asio::ssl::context(method)
			, super(init_buffer_size, max_buffer_size, concurrency)
		{
			// default_workarounds : SSL_OP_ALL
			//	All of the above bug workarounds.
			//	It is usually safe to use SSL_OP_ALL to enable the bug workaround options if compatibility with
			//  somewhat broken implementations is desired.

			// single_dh_use : SSL_OP_SINGLE_DH_USE
			//	Always create a new key when using temporary / ephemeral DH parameters(see ssl_ctx_set_tmp_dh_callback(3)).
			//  This option must be used to prevent small subgroup attacks, when the DH parameters were not generated using
			//  "strong" primes(e.g.when using DSA - parameters, see dhparam(1)).If "strong" primes were used, it is not
			//  strictly necessary to generate a new DH key during each handshake but it is also recommended.SSL_OP_SINGLE_DH_USE
			//  should therefore be enabled whenever temporary / ephemeral DH parameters are used.

			// set default options
			this->set_options(
				asio::ssl::context::default_workarounds |
				asio::ssl::context::no_sslv2 |
				asio::ssl::context::no_sslv3 |
				asio::ssl::context::single_dh_use
			);
		}

		/**
		 * @destructor
		 */
		~tcps_server_impl_t()
		{
			this->stop();
			this->iopool_.stop();
		}

		inline derived_t & set_cert(const std::string& password,
			std::string_view certificate, std::string_view key, std::string_view dh)
		{
			this->set_password_callback([password]
			(std::size_t max_length, asio::ssl::context_base::password_purpose purpose) -> std::string
			{
				return password;
			});

			this->use_certificate_chain(asio::buffer(certificate));
			this->use_private_key(asio::buffer(key), asio::ssl::context::pem);
			this->use_tmp_dh(asio::buffer(dh));

			return (this->derived());
		}

		inline derived_t & set_cert_file(const std::string& password,
			const std::string& certificate, const std::string& key, const std::string& dh)
		{
			this->set_password_callback([password]
			(std::size_t max_length, asio::ssl::context_base::password_purpose purpose) -> std::string
			{
				return password;
			});

			this->use_certificate_chain_file(certificate);
			this->use_private_key_file(key, asio::ssl::context::pem);
			this->use_tmp_dh_file(dh);

			return (this->derived());
		}

	public:
		/**
		 * @function : bind ssl handshake listener
		 * @param    : fun - a user defined callback function
		 * @param    : obj - a pointer or reference to a class object, this parameter can be none
		 * if fun is nonmember function, the obj param must be none, otherwise the obj must be the
		 * the class object's pointer or refrence.
		 * Function signature : void(std::shared_ptr<asio2::tcps_session>& session_ptr, asio2::error_code ec)
		 */
		template<class F, class ...C>
		inline derived_t & bind_handshake(F&& fun, C&&... obj)
		{
			this->listener_.bind(event::handshake,
				observer_t<std::shared_ptr<session_t>&, error_code>(std::forward<F>(fun), std::forward<C>(obj)...));
			return (this->derived());
		}

	protected:
		template<typename... Args>
		inline std::shared_ptr<session_t> _make_session(Args&&... args)
		{
			return super::_make_session(*this, std::forward<Args>(args)...);
		}

	};
}

namespace asio2
{
	template<class session_t>
	class tcps_server_t : public detail::tcps_server_impl_t<tcps_server_t<session_t>, session_t>
	{
	public:
		using detail::tcps_server_impl_t<tcps_server_t<session_t>, session_t>::tcps_server_impl_t;
	};

	using tcps_server = tcps_server_t<tcps_session>;
}

#endif // !__ASIO2_TCPS_SERVER_HPP__

#endif
