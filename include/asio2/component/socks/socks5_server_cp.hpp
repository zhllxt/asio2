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
 */

#ifndef __ASIO2_SOCKS5_SERVER_CP_HPP__
#define __ASIO2_SOCKS5_SERVER_CP_HPP__

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

	template<class SocketT, class Sock5OptT, class CommandCallback>
	class socks5_server_handshake_op : public asio::coroutine
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;

	public:
		SocketT&          socket_;
		Sock5OptT         socks5_;
		CommandCallback   cmd_cb_;

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
		std::uint8_t                     addr_type{}, addr_size{};
		socks5::method                   method{};

		std::uint8_t nmethods{};
		std::uint8_t ulen{};
		std::uint8_t plen{};
		std::string  username{}, password{};
		socks5::command cmd{};
		std::string host{}, port{};

		asio::ip::address bnd_addr{};
		std::uint16_t     bnd_port{};

		asio::steady_timer* connect_finish_timer = nullptr;

		inline bool check_auth()
		{
			std::string username_in_opt = socks5_opt_username(socks5());
			std::string password_in_opt = socks5_opt_password(socks5());

			bool f = false;

			if (!username_in_opt.empty() && !password_in_opt.empty())
			{
				f = (username == username_in_opt && password == password_in_opt);
			}

			if (!f)
			{
				if (auto& authcb = socks5().get_auth_callback(); authcb)
				{
					f = authcb(username, password);
				}
			}

			return f;
		}

		template<class Sock, class S5Opt, class CmdCB>
		socks5_server_handshake_op(Sock& sock, S5Opt&& s5opt, CmdCB&& cmdcb)
			: socket_(sock)
			, socks5_(std::forward<S5Opt>(s5opt))
			, cmd_cb_(std::forward<CmdCB>(cmdcb))
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

				if (nmethods = read<std::uint8_t>(p); nmethods == std::uint8_t(0))
				{
					ec = socks5::make_error_code(socks5::error::no_acceptable_methods);
					goto end;
				}

				stream->consume(stream->size());

				ASIO_CORO_YIELD
					asio::async_read(socket_, strbuf, asio::transfer_exactly(nmethods), std::move(self));
				if (ec)
					goto end;

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

				method = socks5::method::noacceptable;

				for (std::uint8_t i = 0; method == socks5::method::noacceptable && i < nmethods; ++i)
				{
					socks5::method m1 = static_cast<socks5::method>(read<std::uint8_t>(p));

					for(socks5::method m2 : socks5().methods())
					{
						if (m1 == m2)
						{
							method = m1;
							break;
						}
					}
				}

                // +----+--------+
                // |VER | METHOD |
                // +----+--------+
                // | 1  |   1    |
                // +----+--------+

				stream->consume(stream->size());

				bytes  = 2;
				buffer = stream->prepare(bytes);
				p      = static_cast<char*>(buffer.data());

				write(p, std::uint8_t(0x05));                          // VER 
				write(p, std::uint8_t(detail::to_underlying(method))); // METHOD 

				stream->commit(bytes);

				ASIO_CORO_YIELD
					asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
				if (ec)
					goto end;

				if (method == socks5::method::noacceptable)
				{
					ec = socks5::make_error_code(socks5::error::no_acceptable_methods);
					goto end;
				}
				if (method == socks5::method::password)
				{
					//         +----+------+----------+------+----------+
					//         |VER | ULEN |  UNAME   | PLEN |  PASSWD  |
					//         +----+------+----------+------+----------+
					//         | 1  |  1   | 1 to 255 |  1   | 1 to 255 |
					//         +----+------+----------+------+----------+
					
					stream->consume(stream->size());

					ASIO_CORO_YIELD
						asio::async_read(socket_, strbuf, asio::transfer_exactly(1 + 1), std::move(self));
					if (ec)
						goto end;

					p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

					// The VER field contains the current version of the subnegotiation, which is X'01'.
					if (std::uint8_t version = read<std::uint8_t>(p); version != std::uint8_t(0x01))
					{
						ec = socks5::make_error_code(socks5::error::unsupported_authentication_version);
						goto end;
					}

					if (ulen = read<std::uint8_t>(p); ulen == std::uint8_t(0))
					{
						ec = socks5::make_error_code(socks5::error::authentication_failed);
						goto end;
					}

					// read username
					stream->consume(stream->size());

					ASIO_CORO_YIELD
						asio::async_read(socket_, strbuf, asio::transfer_exactly(ulen), std::move(self));
					if (ec)
						goto end;

					p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

					username.assign(p, ulen);

					// read password len
					stream->consume(stream->size());

					ASIO_CORO_YIELD
						asio::async_read(socket_, strbuf, asio::transfer_exactly(1), std::move(self));
					if (ec)
						goto end;

					p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

					if (plen = read<std::uint8_t>(p); plen == std::uint8_t(0))
					{
						ec = socks5::make_error_code(socks5::error::authentication_failed);
						goto end;
					}

					// read password
					stream->consume(stream->size());

					ASIO_CORO_YIELD
						asio::async_read(socket_, strbuf, asio::transfer_exactly(plen), std::move(self));
					if (ec)
						goto end;

					p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

					password.assign(p, plen);

					// compare username and password
					if (!check_auth())
					{
						stream->consume(stream->size());

						bytes  = 2;
						buffer = stream->prepare(bytes);
						p      = static_cast<char*>(buffer.data());

						write(p, std::uint8_t(0x01));                                                        // VER 
						write(p, std::uint8_t(detail::to_underlying(socks5::error::authentication_failed))); // STATUS  

						stream->commit(bytes);

						ASIO_CORO_YIELD
							asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));

						ec = socks5::make_error_code(socks5::error::authentication_failed);
						goto end;
					}
					else
					{
						stream->consume(stream->size());

						bytes  = 2;
						buffer = stream->prepare(bytes);
						p      = static_cast<char*>(buffer.data());

						write(p, std::uint8_t(0x01)); // VER 
						write(p, std::uint8_t(0x00)); // STATUS  

						stream->commit(bytes);

						ASIO_CORO_YIELD
							asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
						if (ec)
							goto end;
					}
				}

				//  +----+-----+-------+------+----------+----------+
				//  |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
				//  +----+-----+-------+------+----------+----------+
				//  | 1  |  1  | X'00' |  1   | Variable |    2     |
				//  +----+-----+-------+------+----------+----------+

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

				// CMD
				cmd = static_cast<socks5::command>(read<std::uint8_t>(p));

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

				bnd_addr = socket_.local_endpoint(ec).address();
				bnd_port = socket_.local_endpoint(ec).port();

				if (host.empty() || port.empty() || std::strtoull(port.data(), nullptr, 10) == 0)
				{
					ec = socks5::make_error_code(socks5::error::host_unreachable);
					goto end;
				}

				stream->consume(stream->size());

				// the address field contains a fully-qualified domain name.  The first
				// octet of the address field contains the number of octets of name that
				// follow, there is no terminating NUL octet.
				buffer = stream->prepare(1 + 1 + 1 + 1 + (std::max)(16, int(host.size() + 1)) + 2);
				p      = static_cast<char*>(buffer.data());

				write(p, std::uint8_t(0x05));      // VER 5.
				write(p, std::uint8_t(0x00));      // REP 
				write(p, std::uint8_t(0x00));      // RSV.

				if (bnd_addr.is_v4())
				{
					write(p, std::uint8_t(0x01)); // ATYP 

					// real length
					bytes = 1 + 1 + 1 + 1 + 4 + 2;

					write(p, bnd_addr.to_v4().to_uint());
				}
				else
				{
					write(p, std::uint8_t(0x04)); // ATYP 

					// real length
					bytes = 1 + 1 + 1 + 1 + 16 + 2;

					auto addr_bytes = bnd_addr.to_v6().to_bytes();
					std::copy(addr_bytes.begin(), addr_bytes.end(), p);
					p += 16;
				}

				// port
				write(p, endpoint.port());

				stream->commit(bytes);

				p = const_cast<char*>(static_cast<const char*>(stream->data().data()));

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

				if (cmd == socks5::command::connect || cmd == socks5::command::udp_associate)
				{
					connect_finish_timer = cmd_cb_(*this);

					if (!connect_finish_timer)
					{
						if (!ec)
							ec = socks5::make_error_code(socks5::error::host_unreachable);
					}
					else
					{
						ASIO_CORO_YIELD
							connect_finish_timer->async_wait(std::move(self));

						ec = get_last_error();
					}

					if (!ec)
						p[1] = char(0x00);
					else if (ec == asio::error::network_unreachable)
						p[1] = char(0x03);
					else if (ec == asio::error::host_unreachable || ec == asio::error::host_not_found)
						p[1] = char(0x04);
					else if (ec == asio::error::connection_refused)
						p[1] = char(0x05);
					else if (ec)
						p[1] = char(0x01);

					ASIO_CORO_YIELD
						asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));
					if (ec)
						goto end;
				}
				else/* if (cmd == socks5::command::bind)*/
				{
					p[1] = char(0x07);

					ASIO_CORO_YIELD
						asio::async_write(socket_, strbuf, asio::transfer_exactly(bytes), std::move(self));

					ec = socks5::make_error_code(socks5::error::command_not_supported);
					goto end;
				}

				ec = {};

			end:
				self.complete(ec);
			}
		}
	};

	// C++17 class template argument deduction guides
	template<class SKT, class S5Opt, class CommandCallback>
	socks5_server_handshake_op(SKT&, S5Opt, CommandCallback) ->
		socks5_server_handshake_op<SKT, S5Opt, CommandCallback>;
}

namespace asio2
{
	/**
	 * @brief Perform the socks5 handshake asynchronously in the client role.
	 * @param socket - The asio::ip::tcp::socket object reference.
	 * @param socks5_opt - The socks5 option, must contains the socks5 proxy server ip and port.
	 * @param cmd_cb - command callback. Signature: asio::steady_timer*(auto& s5_server_handshake_op){}
	 * @param token - The completion handler to invoke when the operation completes.
	 *    The implementation takes ownership of the handler by performing a decay-copy.
	 *	  The equivalent function signature of the handler must be:
	 *    @code
	 *    void handler(
	 *        error_code const& ec    // Result of operation
	 *    );
	 */
	template <typename SocketT, typename Sock5OptT, typename CommandCallback, typename CompletionToken>
	auto socks5_async_handshake(
		SocketT& socket, Sock5OptT&& socks5_opt, CommandCallback&& cmd_cb, CompletionToken&& token)
		-> decltype(asio::async_compose<CompletionToken, void(asio::error_code)>(
			std::declval<detail::socks5_server_handshake_op<SocketT, Sock5OptT, CommandCallback>>(), token, socket))
	{
		return asio::async_compose<CompletionToken, void(asio::error_code)>(
			detail::socks5_server_handshake_op<SocketT, Sock5OptT, CommandCallback>{
			socket, std::forward<Sock5OptT>(socks5_opt), std::forward<CommandCallback>(cmd_cb)},
			token, socket);
	}
}

#endif // !__ASIO2_SOCKS5_SERVER_CP_HPP__
