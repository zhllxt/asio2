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

#ifndef __ASIO2_SSL_CONTEXT_COMPONENT_HPP__
#define __ASIO2_SSL_CONTEXT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <string_view>
#include <filesystem>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isServer>
	class ssl_context_cp : public asio::ssl::context
	{
	public:
		template<typename = void>
		ssl_context_cp(asio::ssl::context::method method)
			: asio::ssl::context(method)
		{
			// default_workarounds : SSL_OP_ALL
			//	All of the above bug workarounds.
			//	It is usually safe to use SSL_OP_ALL to enable the bug workaround options if
			//  compatibility with somewhat broken implementations is desired.

			// single_dh_use : SSL_OP_SINGLE_DH_USE
			//	Always create a new key when using temporary / ephemeral
			//  DH parameters(see ssl_ctx_set_tmp_dh_callback(3)).
			//  This option must be used to prevent small subgroup attacks,
			//  when the DH parameters were not generated using "strong" 
			//  primes(e.g.when using DSA - parameters, see dhparam(1)).
			//  If "strong" primes were used, it is not strictly necessary
			//  to generate a new DH key during each handshake but it is 
			//  also recommended.
			//  SSL_OP_SINGLE_DH_USE should therefore be enabled whenever
			//  temporary / ephemeral DH parameters are used.

			if constexpr (isServer)
			{
				// set default options
				this->set_options(
					asio::ssl::context::default_workarounds |
					asio::ssl::context::no_sslv2 |
					asio::ssl::context::no_sslv3 |
					asio::ssl::context::single_dh_use
				);
			}
			else
			{
				std::ignore = true;
			}
		}

		~ssl_context_cp() = default;

		/**
		 *
		 * >> openssl create your certificates and sign them
		 * ------------------------------------------------------------------------------------------------
		 * // 1. Generate Server private key
		 * openssl genrsa -des3 -out server.key 1024
		 * // 2. Generate Server Certificate Signing Request(CSR)
		 * openssl req -new -key server.key -out server.csr -config openssl.cnf
		 * // 3. Generate Client private key
		 * openssl genrsa -des3 -out client.key 1024
		 * // 4. Generate Client Certificate Signing Request(CSR)
		 * openssl req -new -key client.key -out client.csr -config openssl.cnf
		 * // 5. Generate CA private key
		 * openssl genrsa -des3 -out ca.key 2048
		 * // 6. Generate CA Certificate file
		 * openssl req -new -x509 -key ca.key -out ca.crt -days 3650 -config openssl.cnf
		 * // 7. Generate Server Certificate file
		 * openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
		 * // 8. Generate Client Certificate file
		 * openssl ca -in client.csr -out client.crt -cert ca.crt -keyfile ca.key -config openssl.cnf
		 * // 9. Generate dhparam file
		 * openssl dhparam -out dh1024.pem 1024
		 *
		 */

		 /**
		  * server set_verify_mode :
		  *   "verify_peer", ca_cert_buffer can be empty.
		  *      Whether the client has a certificate or not is ok.
		  *   "verify_fail_if_no_peer_cert", ca_cert_buffer can be empty.
		  *      Whether the client has a certificate or not is ok.
		  *   "verify_peer | verify_fail_if_no_peer_cert", ca_cert_buffer cannot be empty.
		  *      Client must use certificate, otherwise handshake will be failed.
		  * client set_verify_mode :
		  *   "verify_peer", ca_cert_buffer cannot be empty.
		  *   "verify_none", ca_cert_buffer can be empty.
		  * private_cert_buffer,private_key_buffer,private_password always cannot be empty.
		  */
		template<typename = void>
		inline derived_t& set_cert_buffer(
			std::string_view ca_cert_buffer,
			std::string_view private_cert_buffer,
			std::string_view private_key_buffer,
			std::string_view private_password
		)
		{
			this->set_password_callback([password = std::string{ private_password }]
			(std::size_t max_length, asio::ssl::context_base::password_purpose purpose)->std::string
			{
				return password;
			});

			ASIO2_ASSERT(!private_cert_buffer.empty() && !private_key_buffer.empty());

			this->use_certificate(asio::buffer(private_cert_buffer), asio::ssl::context::pem);
			this->use_private_key(asio::buffer(private_key_buffer), asio::ssl::context::pem);

			if (!ca_cert_buffer.empty())
				this->add_certificate_authority(asio::buffer(ca_cert_buffer));

			return (static_cast<derived_t&>(*this));
		}

		/**
		 * server set_verify_mode :
		 *   "verify_peer", ca_cert_buffer can be empty.
		 *      Whether the client has a certificate or not is ok.
		 *   "verify_fail_if_no_peer_cert", ca_cert_buffer can be empty.
		 *      Whether the client has a certificate or not is ok.
		 *   "verify_peer | verify_fail_if_no_peer_cert", ca_cert_buffer cannot be empty.
		 *      Client must use certificate, otherwise handshake will be failed.
		 * client set_verify_mode :
		 *   "verify_peer", ca_cert_buffer cannot be empty.
		 *   "verify_none", ca_cert_buffer can be empty.
		 * private_cert_buffer,private_key_buffer,private_password always cannot be empty.
		 */
		template<typename = void>
		inline derived_t& set_cert_file(
			const std::string& ca_cert_file,
			const std::string& private_cert_file,
			const std::string& private_key_file,
			const std::string& private_password
		)
		{
			std::error_code ec;
			if (!std::filesystem::exists(ca_cert_file, ec) ||
				!std::filesystem::exists(private_cert_file, ec) ||
				!std::filesystem::exists(private_key_file, ec))
			{
				ASIO2_ASSERT(false && "The cert files is not exists.");
				return (static_cast<derived_t&>(*this));
			}

			this->set_password_callback([password = private_password]
			(std::size_t max_length, asio::ssl::context_base::password_purpose purpose)->std::string
			{
				return password;
			});

			ASIO2_ASSERT(!private_cert_file.empty() && !private_key_file.empty());

			this->use_certificate_chain_file(private_cert_file);
			this->use_private_key_file(private_key_file, asio::ssl::context::pem);

			if (!ca_cert_file.empty())
				this->load_verify_file(ca_cert_file);

			return (static_cast<derived_t&>(*this));
		}

		/**
		 * BIO_new_mem_buf -> SSL_CTX_set_tmp_dh
		 */
		inline derived_t& set_dh_buffer(std::string_view dh_buffer)
		{
			if (!dh_buffer.empty())
				this->use_tmp_dh(asio::buffer(dh_buffer));

			return (static_cast<derived_t&>(*this));
		}

		/**
		 * BIO_new_file -> SSL_CTX_set_tmp_dh
		 */
		inline derived_t& set_dh_file(const std::string& dh_file)
		{
			std::error_code ec;
			if (!std::filesystem::exists(dh_file, ec))
			{
				ASIO2_ASSERT(false && "The dh file is not exists.");
				return (static_cast<derived_t&>(*this));
			}

			if (!dh_file.empty())
				this->use_tmp_dh_file(dh_file);

			return (static_cast<derived_t&>(*this));
		}

	protected:
	};
}

#endif // !__ASIO2_SSL_CONTEXT_COMPONENT_HPP__

#endif
