/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * socks5 protocol : https://www.ddhigh.com/2019/08/24/socks5-protocol.html
 * UDP Associate : https://blog.csdn.net/whatday/article/details/40183555
 * http proxy : https://blog.csdn.net/dolphin98629/article/details/54599850
 */

#ifndef __ASIO2_SOCKS5_CLIENT_HPP__
#define __ASIO2_SOCKS5_CLIENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>

#include <memory>
#include <chrono>
#include <functional>
#include <atomic>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/ecs.hpp>

#include <asio2/component/socks/socks5_core.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class SocketT, class Sock5OptT, class HandlerT>
	class socks5_client_connect_op : public asio::coroutine
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		asio::io_context        & context_;

		std::string    host_{}, port_{};

		SocketT&       socket_;
		Sock5OptT      sock5_;
		HandlerT       handler_;

	#if defined(ASIO2_ENABLE_LOG)
		static_assert(detail::is_template_instance_of_v<std::shared_ptr, Sock5OptT>);
	#endif

		std::unique_ptr<asio::streambuf> stream{ std::make_unique<asio::streambuf>() };
		asio::mutable_buffer             buffer{};
		std::size_t                      bytes{};
		char*                            p{};
		asio::ip::address                endpoint{};
		std::string                      username{}, password{};
		std::uint8_t                     addr_type{}, addr_size{};
		socks5::method                   method{};

		template<class SKT, class S5Opt, class H>
		socks5_client_connect_op(
			asio::io_context& context,
			std::string host, std::string port,
			SKT& skt, S5Opt s5, H&& h
		)
			: context_(context)
			, host_   (std::move(host))
			, port_   (std::move(port))
			, socket_ (skt)
			, sock5_  (std::move(s5))
			, handler_(std::forward<H>(h))
		{
			(*this)();
		}

		template<typename = void>
		void operator()(error_code ec = {}, std::size_t bytes_transferred = 0)
		{
			detail::ignore_unused(ec, bytes_transferred);

			asio::streambuf& strbuf = *stream;

			// There is no need to use a timeout timer because there is already has
			// connect_timeout_cp

			ASIO_CORO_REENTER(*this)
			{
				// The client connects to the server, and sends a version
				// identifier / method selection message :

				// +----+----------+----------+
				// |VER | NMETHODS | METHODS  |
				// +----+----------+----------+
				// | 1  |    1     | 1 to 255 |
				// +----+----------+----------+

				stream->consume(stream->size());

				bytes  = 1 + 1 + sock5_->methods_count();
				buffer = stream->prepare(bytes);
				p      = static_cast<char*>(buffer.data());

				write(p, std::uint8_t(0x05));                         // SOCKS VERSION 5.
				write(p, std::uint8_t(sock5_->methods_count()));      // NMETHODS
				for (auto m : sock5_->methods())
				{
					write(p, std::uint8_t(detail::to_underlying(m))); // METHODS
				}

				stream->commit(bytes);

				ASIO_CORO_YIELD
					asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(*this));
				if (ec)
					goto end;

				// The server selects from one of the methods given in METHODS, and 
				// sends a METHOD selection message :

				// +----+--------+
				// |VER | METHOD |
				// +----+--------+
				// | 1  |   1    |
				// +----+--------+

				stream->consume(stream->size());

				ASIO_CORO_YIELD
					asio::async_read(socket_, strbuf, asio::transfer_exactly(1 + 1), std::move(*this));
				if (ec)
					goto end;

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

				if (std::uint8_t version = read<std::uint8_t>(p); version != std::uint8_t(0x05))
				{
					ec = socks5::make_error_code(socks5::error::unsupported_version);
					goto end;
				}

				// If the selected METHOD is X'FF', none of the methods listed by the
				// client are acceptable, and the client MUST close the connection.
				// 
				// The values currently defined for METHOD are:
				// 
				//         o  X'00' NO AUTHENTICATION REQUIRED
				//         o  X'01' GSSAPI
				//         o  X'02' USERNAME/PASSWORD
				//         o  X'03' to X'7F' IANA ASSIGNED
				//         o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
				//         o  X'FF' NO ACCEPTABLE METHODS

				// Once the method-dependent subnegotiation has completed, the client
				// sends the request details.  If the negotiated method includes
				// encapsulation for purposes of integrity checking and/or
				// confidentiality, these requests MUST be encapsulated in the method-
				// dependent encapsulation.
				// 
				// The SOCKS request is formed as follows:
				// 
				//     +----+-----+-------+------+----------+----------+
				//     |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
				//     +----+-----+-------+------+----------+----------+
				//     | 1  |  1  | X'00' |  1   | Variable |    2     |
				//     +----+-----+-------+------+----------+----------+
				// 
				//     Where:
				// 
				//         o  VER    protocol version: X'05'
				//         o  CMD
				//             o  CONNECT X'01'
				//             o  BIND X'02'
				//             o  UDP ASSOCIATE X'03'
				//         o  RSV    RESERVED
				//         o  ATYP   address type of following address
				//             o  IP V4 address: X'01'
				//             o  DOMAINNAME: X'03'
				//             o  IP V6 address: X'04'
				//         o  DST.ADDR       desired destination address
				//         o  DST.PORT desired destination port in network octet
				//             order
				// 
				// The SOCKS server will typically evaluate the request based on source
				// and destination addresses, and return one or more reply messages, as
				// appropriate for the request type.

				method = socks5::method(read<std::uint8_t>(p));

				// cannot use "switch", only "if else" can be used otherwise ASIO_CORO_YIELD will be exception.
				if /**/ (method == socks5::method::anonymous)
				{
				}
				else if (method == socks5::method::gssapi)
				{
					ec = socks5::make_error_code(socks5::error::unsupported_method);
					goto end;
				}
				else if (method == socks5::method::password)
				{
					// Once the SOCKS V5 server has started, and the client has selected the
					// Username/Password Authentication protocol, the Username/Password
					// subnegotiation begins.  This begins with the client producing a
					// Username/Password request:
					// 
					//         +----+------+----------+------+----------+
					//         |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
					//         +----+------+----------+------+----------+
					//         | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
					//         +----+------+----------+------+----------+

					if constexpr (socks5::detail::has_member_username<typename Sock5OptT::element_type>::value)
					{
						username = sock5_->username();
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					if constexpr (socks5::detail::has_member_password<typename Sock5OptT::element_type>::value)
					{
						password = sock5_->password();
					}
					else
					{
						ASIO2_ASSERT(false);
					}

					if (username.empty() && password.empty())
					{
						ASIO2_ASSERT(false);
						ec = socks5::make_error_code(socks5::error::username_required);
						goto end;
					}

					stream->consume(stream->size());

					bytes  = 1 + 1 + username.size() + 1 + password.size();
					buffer = stream->prepare(bytes);
					p      = static_cast<char*>(buffer.data());

					// The VER field contains the current version of the subnegotiation,
					// which is X'01'. The ULEN field contains the length of the UNAME field
					// that follows. The UNAME field contains the username as known to the
					// source operating system. The PLEN field contains the length of the
					// PASSWD field that follows. The PASSWD field contains the password
					// association with the given UNAME.

					// VER
					write(p, std::uint8_t(0x01));

					// ULEN
					write(p, std::uint8_t(username.size()));

					// UNAME
					std::copy(username.begin(), username.end(), p);
					p += username.size();

					// PLEN
					write(p, std::uint8_t(password.size()));

					// PASSWD
					std::copy(password.begin(), password.end(), p);
					p += password.size();

					stream->commit(bytes);

					// send username and password to server
					ASIO_CORO_YIELD
						asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(*this));
					if (ec)
						goto end;

					// The server verifies the supplied UNAME and PASSWD, and sends the
					// following response:
					// 
					//                     +----+--------+
					//                     |VER | STATUS |
					//                     +----+--------+
					//                     | 1  |   1    |
					//                     +----+--------+
					// 
					// A STATUS field of X'00' indicates success. If the server returns a
					// `failure' (STATUS value other than X'00') status, it MUST close the
					// connection.

					// read reply
					stream->consume(stream->size());

					ASIO_CORO_YIELD
						asio::async_read(socket_, strbuf, asio::transfer_exactly(1 + 1), std::move(*this));
					if (ec)
						goto end;

					// parse reply
					p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

					if (std::uint8_t ver = read<std::uint8_t>(p); ver != std::uint8_t(0x01))
					{
						ec = socks5::make_error_code(socks5::error::unsupported_authentication_version);
						goto end;
					}

					if (std::uint8_t status = read<std::uint8_t>(p); status != std::uint8_t(0x00))
					{
						ec = socks5::make_error_code(socks5::error::authentication_failed);
						goto end;
					}
				}
				//else if (method == socks5::method::iana)
				//{
				//	ec = socks5::make_error_code(socks5::error::unsupported_method);
				//	goto end;
				//}
				//else if (method == socks5::method::reserved)
				//{
				//	ec = socks5::make_error_code(socks5::error::unsupported_method);
				//	goto end;
				//}
				else if (method == socks5::method::noacceptable)
				{
					ec = socks5::make_error_code(socks5::error::no_acceptable_methods);
					goto end;
				}
				else
				{
					ec = socks5::make_error_code(socks5::error::no_acceptable_methods);
					goto end;
				}

				stream->consume(stream->size());

				// the address field contains a fully-qualified domain name.  The first
				// octet of the address field contains the number of octets of name that
				// follow, there is no terminating NUL octet.
				buffer = stream->prepare(1 + 1 + 1 + 1 + (std::max)(16, int(host_.size() + 1)) + 2);
				p      = static_cast<char*>(buffer.data());

				write(p, std::uint8_t(0x05));                                     // VER 5.
				write(p, std::uint8_t(detail::to_underlying(sock5_->command()))); // CMD CONNECT .
				write(p, std::uint8_t(0x00));                                     // RSV.

				// ATYP
				endpoint = asio::ip::make_address(host_, ec);
				if (ec)
				{
					ASIO2_ASSERT(host_.size() <= std::size_t(0xff));

					// real length
					bytes = 1 + 1 + 1 + 1 + 1 + host_.size() + 2;

					// type is domain
					write(p, std::uint8_t(0x03));

					// domain size
					write(p, std::uint8_t(host_.size()));

					// domain name 
					std::copy(host_.begin(), host_.end(), p);
					p += host_.size();
				}
				else
				{
					if /**/ (endpoint.is_v4())
					{
						// real length
						bytes = 1 + 1 + 1 + 1 + 4 + 2;

						// type is ipv4
						write(p, std::uint8_t(0x01));

						write(p, std::uint32_t(endpoint.to_v4().to_uint()));
					}
					else if (endpoint.is_v6())
					{
						// real length
						bytes = 1 + 1 + 1 + 1 + 16 + 2;

						// type is ipv6
						write(p, std::uint8_t(0x04));

						auto addr_bytes = endpoint.to_v6().to_bytes();
						std::copy(addr_bytes.begin(), addr_bytes.end(), p);
						p += 16;
					}
					else
					{
						ASIO2_ASSERT(false);
					}
				}

				// port
				write(p, std::uint16_t(std::strtoul(port_.data(), nullptr, 10)));

				stream->commit(bytes);

				ASIO_CORO_YIELD
					asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(*this));
				if (ec)
					goto end;

				// The SOCKS request information is sent by the client as soon as it has
				// established a connection to the SOCKS server, and completed the
				// authentication negotiations.  The server evaluates the request, and
				// returns a reply formed as follows:
				// 
				//     +----+-----+-------+------+----------+----------+
				//     |VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
				//     +----+-----+-------+------+----------+----------+
				//     | 1  |  1  | X'00' |  1   | Variable |    2     |
				//     +----+-----+-------+------+----------+----------+
				// 
				//     Where:
				// 
				//         o  VER    protocol version: X'05'
				//         o  REP    Reply field:
				//             o  X'00' succeeded
				//             o  X'01' general SOCKS server failure
				//             o  X'02' connection not allowed by ruleset
				//             o  X'03' Network unreachable
				//             o  X'04' Host unreachable
				//             o  X'05' Connection refused
				//             o  X'06' TTL expired
				//             o  X'07' Command not supported
				//             o  X'08' Address type not supported
				//             o  X'09' to X'FF' unassigned
				//         o  RSV    RESERVED
				//         o  ATYP   address type of following address

				stream->consume(stream->size());

				// 1. read the first 5 bytes : VER REP RSV ATYP [LEN]
				ASIO_CORO_YIELD
					asio::async_read(socket_, strbuf, asio::transfer_exactly(5), std::move(*this));
				if (ec)
					goto end;

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

				// VER
				if (std::uint8_t ver = read<std::uint8_t>(p); ver != std::uint8_t(0x05))
				{
					ec = socks5::make_error_code(socks5::error::unsupported_version);
					goto end;
				}

				// REP
				switch (read<std::uint8_t>(p))
				{
				case std::uint8_t(0x00): ec = {}													; break;
				case std::uint8_t(0x02): ec = asio::error::no_permission                            ; break;
				case std::uint8_t(0x03): ec = asio::error::network_unreachable                      ; break;
				case std::uint8_t(0x04): ec = asio::error::host_unreachable                         ; break;
				case std::uint8_t(0x05): ec = asio::error::connection_refused                       ; break;
				case std::uint8_t(0x06): ec = asio::error::timed_out                                ; break;
				case std::uint8_t(0x08): ec = asio::error::address_family_not_supported             ; break;
				case std::uint8_t(0x01): ec = socks5::make_error_code(socks5::error::general_socks_server_failure)	; break;
				case std::uint8_t(0x07): ec = socks5::make_error_code(socks5::error::command_not_supported)			; break;
				case std::uint8_t(0x09): ec = socks5::make_error_code(socks5::error::unassigned)                    ; break;
				default:                 ec = socks5::make_error_code(socks5::error::unassigned)                    ; break;
				}

				if (ec)
					goto end;

				// skip RSV.
				read<std::uint8_t>(p);

				addr_type = read<std::uint8_t>(p); // ATYP
				addr_size = read<std::uint8_t>(p); // [LEN]

				// ATYP
				switch (addr_type)
				{
				case std::uint8_t(0x01): bytes = 4         + 2 - 1; break; // IP V4 address: X'01'
				case std::uint8_t(0x03): bytes = addr_size + 2 - 0; break; // DOMAINNAME: X'03'
				case std::uint8_t(0x04): bytes = 16        + 2 - 1; break; // IP V6 address: X'04'
				default:
				{
					ec = socks5::make_error_code(socks5::error::general_failure);
					goto end;
				}
				}

				stream->consume(stream->size());

				ASIO_CORO_YIELD
					asio::async_read(socket_, strbuf, asio::transfer_exactly(bytes), std::move(*this));
				if (ec)
					goto end;

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

				switch (addr_type)
				{
				case std::uint8_t(0x01): // IP V4 address: X'01'
				{
					std::uint8_t addr[4]{ 0 };
					addr[0] = addr_size;
					addr[1] = read<std::uint8_t>(p);
					addr[2] = read<std::uint8_t>(p);
					addr[3] = read<std::uint8_t>(p);
					if (is_little_endian())
					{
						swap_bytes<4>(reinterpret_cast<std::uint8_t *>(addr));
					}
					std::uint16_t port = read<std::uint16_t>(p);
					detail::ignore_unused(addr, port);
				}
				break;
				case std::uint8_t(0x03): // DOMAINNAME: X'03'
				{
					std::string addr;
					addr.resize(addr_size);
					std::copy(p, p + addr_size, addr.data());
					p += addr_size;
					std::uint16_t port = read<std::uint16_t>(p);
					detail::ignore_unused(addr, port);
				}
				break;
				case std::uint8_t(0x04): // IP V6 address: X'04'
				{
					std::uint8_t addr[16]{ 0 };
					addr[0] = addr_size;
					for (int i = 1; i < 16; i++)
					{
						addr[i] = read<std::uint8_t>(p);
					}
					std::uint16_t port = read<std::uint16_t>(p);
					detail::ignore_unused(addr, port);
				}
				break;
				}

				ec = {};

			end:
				handler_(ec);
			}
		}
	};

	// C++17 class template argument deduction guides
	template<class SKT, class S5Opt, class H>
	socks5_client_connect_op(asio::io_context&, std::string, std::string,
		SKT&, S5Opt, H)->socks5_client_connect_op<SKT, S5Opt, H>;

	template<class derived_t, class args_t>
	class socks5_client_impl
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:

		/**
		 * @brief constructor
		 */
		socks5_client_impl() {}

		/**
		 * @brief destructor
		 */
		~socks5_client_impl() = default;

	protected:
		template<typename C>
		inline void _socks5_init(std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);
		}

		template<typename C, typename DeferEvent>
		inline void _socks5_start(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (ecs_helper::has_socks5<C>())
			{
				socks5_client_connect_op
				{
					derive.io().context(),
					derive.host_, derive.port_,
					derive.socket(),
					ecs->get_component().socks5_option(std::in_place),
					[this, this_ptr, ecs, chain = std::move(chain)](error_code ec) mutable
					{
						derived_t& derive = static_cast<derived_t&>(*this);

						derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					}
				};
			}
			else
			{
				ASIO2_ASSERT(!get_last_error());
				derive._handle_proxy(error_code{}, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		inline void _socks5_stop()
		{
		}
	};
}

#endif // !__ASIO2_SOCKS5_CLIENT_HPP__
