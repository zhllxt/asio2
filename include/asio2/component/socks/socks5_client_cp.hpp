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

#ifndef __ASIO2_SOCKS5_CLIENT_CP_HPP__
#define __ASIO2_SOCKS5_CLIENT_CP_HPP__

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
#include <asio2/component/socks/socks5_util.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;

	template<class SocketT, class Sock5OptT>
	class socks5_client_handshake_op : public asio::coroutine
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		std::string    host_{}, port_{};

		SocketT&       socket_;
		Sock5OptT      socks5_;

		using socks5_value_type = typename detail::element_type_adapter<Sock5OptT>::type;

		inline socks5_value_type& socks5() noexcept
		{
			return detail::to_element_ref(socks5_);
		}

		template<class S5Opt>
		inline std::string socks5_opt_username(S5Opt& s5opt) noexcept
		{
			if constexpr (socks5::detail::has_member_username<S5Opt>::value)
				return s5opt.username();
			else
				return "";
		}

		template<class S5Opt>
		inline std::string socks5_opt_password(S5Opt& s5opt) noexcept
		{
			if constexpr (socks5::detail::has_member_password<S5Opt>::value)
				return s5opt.password();
			else
				return "";
		}

		std::unique_ptr<asio::streambuf> stream{ std::make_unique<asio::streambuf>() };
		asio::mutable_buffer             buffer{};
		std::size_t                      bytes{};
		char*                            p{};
		asio::ip::tcp::endpoint          endpoint{};
		std::string                      username{}, password{};
		std::uint8_t                     addr_type{}, addr_size{};
		socks5::method                   method{};
		std::string                      host{}, port{};

		template<class SKT, class S5Opt>
		socks5_client_handshake_op(std::string host, std::string port, SKT& skt, S5Opt s5)
			: host_   (std::move(host))
			, port_   (std::move(port))
			, socket_ (skt)
			, socks5_  (std::move(s5))
		{
		}

		template <typename Self>
		void operator()(Self& self, error_code ec = {}, std::size_t bytes_transferred = 0)
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

				bytes  = 1 + 1 + socks5().methods_count();
				buffer = stream->prepare(bytes);
				p      = static_cast<char*>(buffer.data());

				write(p, std::uint8_t(0x05));                         // SOCKS VERSION 5.
				write(p, std::uint8_t(socks5().methods_count()));     // NMETHODS
				for (auto m : socks5().methods())
				{
					write(p, std::uint8_t(detail::to_underlying(m))); // METHODS
				}

				stream->commit(bytes);

				ASIO_CORO_YIELD
					asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
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
					asio::async_read(socket_, strbuf, asio::transfer_exactly(1 + 1), std::move(self));
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

					username = socks5_opt_username(socks5());
					password = socks5_opt_password(socks5());

					if (username.empty() || password.empty())
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
						asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
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
						asio::async_read(socket_, strbuf, asio::transfer_exactly(1 + 1), std::move(self));
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

				write(p, std::uint8_t(0x05));                                      // VER 5.
				write(p, std::uint8_t(detail::to_underlying(socks5().command()))); // CMD CONNECT .
				write(p, std::uint8_t(0x00));                                      // RSV.

				// ATYP
				endpoint.address(asio::ip::make_address(host_, ec));
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
					if /**/ (endpoint.address().is_v4())
					{
						// real length
						bytes = 1 + 1 + 1 + 1 + 4 + 2;

						// type is ipv4
						write(p, std::uint8_t(0x01));

						write(p, std::uint32_t(endpoint.address().to_v4().to_uint()));
					}
					else if (endpoint.address().is_v6())
					{
						// real length
						bytes = 1 + 1 + 1 + 1 + 16 + 2;

						// type is ipv6
						write(p, std::uint8_t(0x04));

						auto addr_bytes = endpoint.address().to_v6().to_bytes();
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
					asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
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
					asio::async_read(socket_, strbuf, asio::transfer_exactly(5), std::move(self));
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
				case std::uint8_t(0x00): ec = {}													                   ; break;
				case std::uint8_t(0x01): ec = socks5::make_error_code(socks5::error::general_socks_server_failure     ); break;
				case std::uint8_t(0x02): ec = socks5::make_error_code(socks5::error::connection_not_allowed_by_ruleset); break;
				case std::uint8_t(0x03): ec = socks5::make_error_code(socks5::error::network_unreachable              ); break;
				case std::uint8_t(0x04): ec = socks5::make_error_code(socks5::error::host_unreachable                 ); break;
				case std::uint8_t(0x05): ec = socks5::make_error_code(socks5::error::connection_refused               ); break;
				case std::uint8_t(0x06): ec = socks5::make_error_code(socks5::error::ttl_expired                      ); break;
				case std::uint8_t(0x07): ec = socks5::make_error_code(socks5::error::command_not_supported            ); break;
				case std::uint8_t(0x08): ec = socks5::make_error_code(socks5::error::address_type_not_supported       ); break;
				case std::uint8_t(0x09): ec = socks5::make_error_code(socks5::error::unassigned                       ); break;
				default:                 ec = socks5::make_error_code(socks5::error::unassigned                       ); break;
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
					ec = socks5::make_error_code(socks5::error::address_type_not_supported);
					goto end;
				}
				}

				stream->consume(stream->size());

				ASIO_CORO_YIELD
					asio::async_read(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
				if (ec)
					goto end;

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

				switch (addr_type)
				{
				case std::uint8_t(0x01): // IP V4 address: X'01'
				{
					asio::ip::address_v4::bytes_type addr{};
					addr[0] = addr_size;
					addr[1] = read<std::uint8_t>(p);
					addr[2] = read<std::uint8_t>(p);
					addr[3] = read<std::uint8_t>(p);
					try
					{
						endpoint.address(asio::ip::address_v4(addr));
						host = endpoint.address().to_string();
					}
					catch (const system_error&)
					{
					}
					std::uint16_t uport = read<std::uint16_t>(p);
					endpoint.port(uport);
					port = std::to_string(uport);
				}
				break;
				case std::uint8_t(0x03): // DOMAINNAME: X'03'
				{
					std::string addr;
					addr.resize(addr_size);
					std::copy(p, p + addr_size, addr.data());
					p += addr_size;
					host = std::move(addr);
					std::uint16_t uport = read<std::uint16_t>(p);
					endpoint.port(uport);
					port = std::to_string(uport);
				}
				break;
				case std::uint8_t(0x04): // IP V6 address: X'04'
				{
					asio::ip::address_v6::bytes_type addr{};
					addr[0] = addr_size;
					for (int i = 1; i < 16; i++)
					{
						addr[i] = read<std::uint8_t>(p);
					}
					try
					{
						endpoint.address(asio::ip::address_v6(addr));
						host = endpoint.address().to_string();
					}
					catch (const system_error&)
					{
					}
					std::uint16_t uport = read<std::uint16_t>(p);
					endpoint.port(uport);
					port = std::to_string(uport);
				}
				break;
				}

				ec = {};

			end:
				self.complete(ec, std::move(host), std::move(port));
			}
		}
	};

	// C++17 class template argument deduction guides
	template<class SKT, class S5Opt>
	socks5_client_handshake_op(std::string, std::string, SKT&, S5Opt) -> socks5_client_handshake_op<SKT, S5Opt>;
}

namespace asio2
{
	/**
	 * @brief Perform the socks5 handshake asynchronously in the client role.
	 * @param host - The target server ip. note: not the socks5 proxy server ip.
	 * @param port - The target server port. note: not the socks5 proxy server port.
	 * @param socket - The asio::ip::tcp::socket object reference.
	 * @param socks5_opt - The socks5 option, must contains the socks5 proxy server ip and port.
	 * @param token - The completion handler to invoke when the operation completes. 
	 *    The implementation takes ownership of the handler by performing a decay-copy.
	 *	  The equivalent function signature of the handler must be:
     *    @code
     *    void handler(error_code const& ec, std::string host, std::string port);
	 */
	template <typename SocketT, typename Sock5OptT, typename CompletionToken>
	auto socks5_async_handshake(
		std::string host, std::string port, SocketT& socket, Sock5OptT socks5_opt, CompletionToken&& token)
		-> decltype(asio::async_compose<CompletionToken, void(asio::error_code, std::string, std::string)>(
			std::declval<detail::socks5_client_handshake_op<SocketT, Sock5OptT>>(), token, socket))
	{
		return asio::async_compose<CompletionToken, void(asio::error_code, std::string, std::string)>(
			detail::socks5_client_handshake_op<SocketT, Sock5OptT>{
			std::move(host), std::move(port), socket, std::move(socks5_opt)},
			token, socket);
	}
}

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class socks5_client_cp_impl
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
		socks5_client_cp_impl() {}

		/**
		 * @brief destructor
		 */
		~socks5_client_cp_impl() = default;

	protected:
		template<typename C>
		inline void _check_socks5_command(std::shared_ptr<derived_t>&, std::shared_ptr<ecs_t<C>>& ecs)
		{
			if (!ecs)
				return;

			auto s5opt = ecs->get_component().socks5_option(std::in_place);

			if (s5opt && static_cast<int>(s5opt->command()) == 0)
			{
				if (std::is_base_of_v<detail::tcp_tag, derived_t>)
				{
					s5opt->command(socks5::command::connect);
				}
				else if (std::is_base_of_v<detail::udp_tag, derived_t>)
				{
					s5opt->command(socks5::command::udp_associate);
				}
			}
		}
	};

	template<class derived_t, class args_t, typename TagType = typename args_t::tl_tag_type>
	class socks5_client_cp_bridge {};

	template<class derived_t, class args_t>
	class socks5_client_cp_bridge<derived_t, args_t, detail::tcp_tag>
		: public socks5_client_cp_impl<derived_t, args_t>
	{
		template<class, class = void>
		struct has_member_set_bnd_addr : std::false_type {};

		template<class T>
		struct has_member_set_bnd_addr<T, std::void_t<decltype(
			std::declval<std::decay_t<T>&>().set_bnd_addr(std::string{}))>> : std::true_type {};

		template<class, class = void>
		struct has_member_set_bnd_port : std::false_type {};

		template<class T>
		struct has_member_set_bnd_port<T, std::void_t<decltype(
			std::declval<std::decay_t<T>&>().set_bnd_port(std::string{}))>> : std::true_type {};

	protected:
		// this function name can't be set_bnd_addr, it maybe cause recursive deadloop
		// when use udp client with socks5 option, the udp client will create a tcp client
		// to connect to the socks5 server, and the tcp client will use the same socks5 option
		// as the udp client's too, so the tcp client will derived from current class again,
		// so at here, the tparam D is the tcp client with socks5 option which is the internal
		// tcp connection of the udp client.
		template<class D = derived_t>
		inline void do_set_bnd_addr(std::string addr)
		{
			D& derive = static_cast<D&>(*this);

			if constexpr (has_member_set_bnd_addr<D>::value)
				derive.set_bnd_addr(std::move(addr));
			else
				detail::ignore_unused(addr);
		}
		// this function name can't be set_bnd_port, it maybe cause recursive deadloop
		template<class D = derived_t>
		inline void do_set_bnd_port(std::string port)
		{
			D& derive = static_cast<D&>(*this);

			if constexpr (has_member_set_bnd_port<D>::value)
				derive.set_bnd_port(std::move(port));
			else
				detail::ignore_unused(port);
		}

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
				this->_check_socks5_command(this_ptr, ecs);

				socks5_async_handshake
				(
					derive.host_, derive.port_,
					derive.socks5_socket(),
					ecs->get_component().socks5_option(std::in_place),
					[this, this_ptr, ecs, chain = std::move(chain)]
					(error_code ec, std::string host, std::string port) mutable
					{
						derived_t& derive = static_cast<derived_t&>(*this);

						ASIO2_ASSERT(derive.running_in_this_thread());

						this->do_set_bnd_addr(std::move(host));
						this->do_set_bnd_port(std::move(port));

						derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					}
				);
			}
			else
			{
				ASIO2_ASSERT(!get_last_error());
				derive._handle_proxy(error_code{}, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		inline void _socks5_stop() noexcept
		{
		}

	public:
		inline auto& socks5_socket() noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);
			return derive.socket();
		}

		inline const auto& socks5_socket() const noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);
			return derive.socket();
		}
	};

	template<class derived_t, class args_t>
	class socks5_client_cp_bridge<derived_t, args_t, detail::udp_tag>
		: public socks5_client_cp_impl<derived_t, args_t>
	{
	protected:
		class internal_socks5_client_impl : public args_t::template socks5_client_t<internal_socks5_client_impl>
		{
			friend derived_t;
			template<class, class, typename> friend class socks5_client_cp_bridge;
			template<class, class> friend class connect_cp;

		public:
			using super = typename args_t::template socks5_client_t<internal_socks5_client_impl>;

			template<class... Args>
			internal_socks5_client_impl(Args&&... args)
				: super(std::forward<Args>(args)...)
			{
			}

			template<class T>
			inline auto data_filter_before_send(T&& data)
			{
				std::string_view sv = asio2::to_string_view(asio::buffer(data));

				// can't write the head at here, beacuse there maybe has multi thread call this function.
				std::vector<std::uint8_t>& head = this->udp_data_head_;

				std::uint16_t udatalen = detail::host_to_network(std::uint16_t(sv.size()));
				std::uint8_t* pdatalen = reinterpret_cast<std::uint8_t*>(std::addressof(udatalen));

				// the data is: stds::string, std::vector...
				if constexpr (detail::has_member_insert<T>::value && !std::is_const_v<std::remove_reference_t<T>>)
				{
					data.insert(data.cbegin(), head.begin(), head.end());
					std::memcpy((void*)(data.cbegin().operator->()), (const void*)pdatalen, sizeof(std::uint16_t));
					return std::forward<T>(data);
				}
				else
				{
					std::string str{sv};
					str.insert(str.cbegin(), head.begin(), head.end());
					str[0] = std::string::value_type(pdatalen[0]);
					str[1] = std::string::value_type(pdatalen[1]);
					return str;
				}
			}

			// recvd data from the socks5 server by tcp.
			// cant remove the socks5 protocol head, beacuse when this bind_recv is called,
			// it will call the udp client's _fire_recv, and the _fire_recv will remove the
			// socks5 protocol head by itself.
			//inline std::string_view data_filter_before_recv(std::string_view data)
			//{
			//	auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, true);

			//	detail::ignore_unused(err, ep, domain, real_data);

			//	return real_data;
			//}

			void set_udp_data_head(const std::string& host, const std::string& port)
			{
				std::vector<std::uint8_t>& head = this->udp_data_head_;

				head.reserve(4 + 4 + 2);

				head.push_back(std::uint8_t(0x00)); // RSV 
				head.push_back(std::uint8_t(0x00)); // RSV 
				head.push_back(std::uint8_t(0x00)); // FRAG 

				error_code ec{};
				asio::ip::address addr = asio::ip::address::from_string(host, ec);

				if (ec) // DOMAINNAME: X'03'
				{
					head.push_back(std::uint8_t(0x03));
					head.push_back(std::uint8_t(host.size()));
					head.insert(head.cend(), host.begin(), host.end());
				}
				else if (addr.is_v4()) // IP V4 address: X'01'
				{
					head.push_back(std::uint8_t(0x01));
					asio::ip::address_v4::bytes_type bytes = addr.to_v4().to_bytes();
					head.insert(head.cend(), bytes.begin(), bytes.end());
				}
				else if (addr.is_v6()) // IP V6 address: X'04'
				{
					head.push_back(std::uint8_t(0x04));
					asio::ip::address_v6::bytes_type bytes = addr.to_v6().to_bytes();
					head.insert(head.cend(), bytes.begin(), bytes.end());
				}

				std::uint16_t uport = detail::host_to_network(std::uint16_t(std::strtoul(port.data(), nullptr, 10)));
				std::uint8_t* pport = reinterpret_cast<std::uint8_t*>(std::addressof(uport));
				head.push_back(pport[0]);
				head.push_back(pport[1]);
			}

			inline void set_bnd_addr(std::string addr)
			{
				bnd_addr_ = std::move(addr);
			}
			inline void set_bnd_port(std::string port)
			{
				bnd_port_ = std::move(port);
			}

		protected:
			template<typename C, typename DeferEvent>
			inline void _post_proxy(
				const error_code& ec,
				std::shared_ptr<internal_socks5_client_impl> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
			{
				// after _post_proxy, the socks5_async_handshake will be called, and the socks5_async_handshake
				// will use the derive.host_ and derive.port_ to as the DST.ADDR and DST.PORT of the 
				// socks5 protocol. and this is udp, so we need set the DST.PORT to the udp client's
				// local port.
				this->set_port(this->dst_port_);

				ASIO2_ASSERT(this->running_in_this_thread());

				super::_post_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
			}

		protected:
			std::uint16_t dst_port_{};

			std::string bnd_addr_{};
			std::string bnd_port_{};

			std::vector<std::uint8_t> udp_data_head_{};
		};

	protected:
		template<typename C>
		inline void _socks5_init(std::shared_ptr<ecs_t<C>>& ecs)
		{
			detail::ignore_unused(ecs);

			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (ecs_helper::has_socks5<C>())
				this->socks5_client_ = std::make_shared<internal_socks5_client_impl>(derive.io_->context());
			else
				std::ignore = true;
		}

		template<typename C, typename DeferEvent>
		inline void _socks5_start(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (ecs_helper::has_socks5<C>())
			{
				this->_check_socks5_command(this_ptr, ecs);

				auto s5opt = ecs->get_component().socks5_option(std::in_place);

				this->socks5_client_->bind_connect([]() mutable
				{
				}).bind_disconnect([&derive, wptr = derive.weak_from_this()]() mutable
				{
					// socks5 connection is disconnected, so close the udp client too.
					ASIO2_ASSERT(derive.running_in_this_thread());
					derive._do_disconnect(asio2::get_last_error(), wptr.lock());
				}).bind_recv([&derive, wptr = derive.weak_from_this(), ecs](std::string_view data) mutable
				{
					// recvd data from the socks5 server by tcp, forward the data to the udp client.
					ASIO2_ASSERT(derive.running_in_this_thread());
					std::shared_ptr<derived_t> this_ptr = wptr.lock();
					derive._fire_recv(this_ptr, ecs, data);
				});

				// save this udp client's local bind port.
				this->socks5_client_->dst_port_ = derive.get_local_port();

				// disable the auto reconnect
				this->socks5_client_->set_auto_reconnect(false);

				this->socks5_client_->async_start(s5opt->host(), s5opt->port(),
				[this, &derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
				(error_code ec) mutable
				{
					ASIO2_ASSERT(derive.running_in_this_thread());

					if (ec)
					{
						derive._handle_proxy(ec,
							std::move(this_ptr), std::move(ecs), std::move(chain));
						return;
					}

					this->socks5_client_->set_udp_data_head(derive.host_, derive.port_);

					auto s5opt = ecs->get_component().socks5_option(std::in_place);

					asio::ip::address addr = asio::ip::address::from_string(this->socks5_client_->bnd_addr_, ec);
					if (ec)
					{
						this->socks5_client_->bnd_addr_ = s5opt->host();
					}
					else
					{
						if (addr.is_unspecified())
							this->socks5_client_->bnd_addr_ = s5opt->host();
					}

					derive._reconnect_to_socks5_server(std::move(this_ptr), std::move(ecs), std::move(chain));

				}, detail::socks5_udp_match_role, s5opt);
			}
			else
			{
				ASIO2_ASSERT(!get_last_error());
				derive._handle_proxy(error_code{}, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
		}

		inline void _socks5_stop()
		{
			if (this->socks5_client_)
			{
				this->socks5_client_->stop();

				// if we destroy this shared_ptr at here, it maybe cause crash, beacuse the 
				// data_filter_before_send of this class maybe called agagin after it destroyed, 
				// and it will read the head variable which is in the socks5 client, so it
				// will cause crash.

				//this->socks5_client_.reset();
			}
		}

		template<typename C, typename DeferEvent>
		inline void _reconnect_to_socks5_server(
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			if constexpr (std::is_base_of_v<detail::cast_tag, derived_t>)
			{
				ASIO2_ASSERT(derive.running_in_this_thread());
				derive._handle_proxy(error_code{}, std::move(this_ptr), std::move(ecs), std::move(chain));
			}
			else
			{
				asio2::async_connect(
					this->socks5_client_->bnd_addr_,
					this->socks5_client_->bnd_port_,
					derive.socket(),
				[&derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
				(const error_code& ec) mutable
				{
					ASIO2_ASSERT(derive.running_in_this_thread());
					derive._handle_proxy(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
				});
			}
		}

		// send data to the socks5 server by udp, we need add socks5 protocol head before the data.
		template<class T>
		inline auto data_filter_before_send(T&& data)
		{
			// the data is: stds::string, std::vector...
			if constexpr (detail::has_member_insert<T>::value && !std::is_const_v<std::remove_reference_t<T>>)
			{
				if (this->socks5_client_ == nullptr)
					return std::forward<T>(data);

				std::vector<std::uint8_t>& head = this->socks5_client_->udp_data_head_;
				data.insert(data.cbegin(), head.begin(), head.end());
				return std::forward<T>(data);
			}
			else
			{
				std::string str{asio2::to_string_view(asio::buffer(data))};

				if (this->socks5_client_ == nullptr)
					return str;

				std::vector<std::uint8_t>& head = this->socks5_client_->udp_data_head_;
				str.insert(str.cbegin(), head.begin(), head.end());
				return str;
			}
		}

		// recvd data from the socks5 server by udp, we need remove the socks5 protocol head.
		inline std::string_view data_filter_before_recv(std::string_view data)
		{
			if (this->socks5_client_ == nullptr)
				return data;

			auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, false);

			detail::ignore_unused(err, ep, domain, real_data);

			return real_data;
		}

	public:
		inline auto& socks5_socket() noexcept
		{
			return this->socks5_client_->socket();
		}

		inline const auto& socks5_socket() const noexcept
		{
			return this->socks5_client_->socket();
		}

		inline std::shared_ptr<internal_socks5_client_impl> get_socks5_connection() noexcept
		{
			return this->socks5_client_;
		}

		//template<class... Args>
		//inline void async_send_by_socks5_connection(Args&&... args) noexcept
		//{
		//	ASIO2_ASSERT(this->socks5_client_ != nullptr);

		//	if (this->socks5_client_)
		//	{
		//		this->socks5_client_->async_send(std::forward<Args>(args)...);
		//	}
		//}

	protected:
		std::shared_ptr<internal_socks5_client_impl> socks5_client_;
	};

	template<class derived_t, class args_t>
	class socks5_client_cp : public socks5_client_cp_bridge<derived_t, args_t> {};
}

#endif // !__ASIO2_SOCKS5_CLIENT_CP_HPP__
