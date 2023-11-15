/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_SESSION_HPP__
#define __ASIO2_SOCKS5_SESSION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <variant>

#include <asio2/base/detail/push_options.hpp>

#include <asio2/tcp/tcp_session.hpp>
#include <asio2/component/socks/socks5_server_cp.hpp>
#include <asio2/proxy/socks5_transfer.hpp>

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SERVER;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class derived_t, class args_t = template_args_tcp_session>
	class socks5_session_impl_t
		: public tcp_session_impl_t<derived_t, args_t>
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SERVER;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using super = tcp_session_impl_t   <derived_t, args_t>;
		using self  = socks5_session_impl_t<derived_t, args_t>;

		using args_type   = args_t;
		using key_type    = std::size_t;
		using buffer_type = typename args_t::buffer_t;

	protected:
		using super::send;
		using super::async_send;

	public:
		/**
		 * @brief constructor
		 */
		template<class... Args>
		explicit socks5_session_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		{
		}

		/**
		 * @brief destructor
		 */
		~socks5_session_impl_t()
		{
		}

	public:
		/**
		 * @brief destroy the content of all member variables, this is used for solve the memory leaks.
		 * After this function is called, this class object cannot be used again.
		 */
		inline void destroy()
		{
			derived_t& derive = this->derived();

			std::visit([](auto& ptr) mutable
			{
				ptr.reset();
			}, this->back_client_);

			derive.socks5_options_ = {};

			super::destroy();
		}

		/**
		 * @brief get this object hash key,used for session map
		 */
		inline key_type hash_key() const noexcept
		{
			return reinterpret_cast<key_type>(this);
		}

		/**
		 * @brief Set the socks5 options.
		 */
		inline derived_t& set_socks5_options(socks5::options socks5_options)
		{
			this->socks5_options_ = std::move(socks5_options);
			return this->derived();
		}

		/**
		 * @brief Set the socks5 options.
		 */
		inline socks5::options& get_socks5_options() noexcept
		{
			return this->socks5_options_;
		}

		/**
		 * @brief Get the socks5 options.
		 */
		inline const socks5::options& get_socks5_options() const noexcept
		{
			return this->socks5_options_;
		}

		/**
		 * @brief Get the front client type. tcp or udp.
		 */
		inline asio2::net_protocol get_front_client_type() const noexcept
		{
			switch (this->back_client_.index())
			{
			case 0: return asio2::net_protocol::tcp;
			case 1: return asio2::net_protocol::udp;
			default:return asio2::net_protocol::none;
			}
		}

	protected:
		template<class T>
		inline asio::steady_timer* _handle_socks5_command(
			std::shared_ptr<derived_t> session_ptr, T& s5_server_handshake_op)
		{
			ASIO2_ASSERT(session_ptr->running_in_this_thread());

			auto host = s5_server_handshake_op.host;
			auto port = s5_server_handshake_op.port;

			if (s5_server_handshake_op.cmd == socks5::command::connect)
			{
				auto back_client = std::make_shared<asio2::socks5_tcp_transfer>(this->io_);

				back_client->bind_connect([]() mutable
				{
					set_last_error(get_last_error());
				}).bind_disconnect([session_ptr]() mutable
				{
					// back client has disconnected, so we need disconnect the front client too.
					ASIO2_ASSERT(session_ptr->running_in_this_thread());
					session_ptr->stop();
				}).bind_recv([session_ptr](std::string_view data) mutable
				{
					// tcp: recvd data from the back client, forward the data to the front client.
					ASIO2_ASSERT(session_ptr->running_in_this_thread());
					session_ptr->async_send(data);
				});

				// async start the back client.
				if (!back_client->async_start(std::move(host), std::move(port)))
					return nullptr;

				// set timer expire, the socks5 async handshake will wait this timer by coroutine.
				back_client->connect_finish_timer_->expires_after(std::chrono::steady_clock::duration::max());

				this->back_client_ = back_client;

				return back_client->connect_finish_timer_.get();
			}
			else if (s5_server_handshake_op.cmd == socks5::command::udp_associate)
			{
				auto back_client = std::make_shared<asio2::socks5_udp_transfer>(this->io_);

				back_client->bind_start(
				[strbuf = s5_server_handshake_op.stream.get(), pback_client = back_client.get()]() mutable
				{
					set_last_error(get_last_error());

					// when code run to here, the socks5 async handshake is waiting the 
					// connect finished timer.
					// when this callback returned, the connect finished timer will be canceled.
					// so timer waiter of the socks5 async handshake will be returned too.

					// we need reset the BND.PORT as the local binded port, beacuse the 
					// BND.PORT will be sent to the front client, and the front client 
					// will sent data to this BND.PORT

					std::uint16_t uport = pback_client->get_local_port();

					auto addr = pback_client->socket().local_endpoint(get_last_error()).address();

					char* p = const_cast<char*>(static_cast<const char*>(strbuf->data().data()));

					if /**/ (addr.is_v4())
					{
						p += 1 + 1 + 1 + 1 + 4;

						detail::write(p, uport);
					}
					else
					{
						p += 1 + 1 + 1 + 1 + 16;

						detail::write(p, uport);
					}
				}).bind_stop([session_ptr]() mutable
				{
					// back client has stoped, so we need disconnect the front client too.
					ASIO2_ASSERT(session_ptr->running_in_this_thread());
					session_ptr->stop();
				}).bind_recv([this, wptr = std::weak_ptr<asio2::socks5_udp_transfer>(back_client)]
				(asio::ip::udp::endpoint& endpoint, std::string_view data) mutable
				{
					// udp: recvd data, maybe from front client or back client, 
					// we need to check whether from front or back by ip and port.
					ASIO2_ASSERT(this->running_in_this_thread());
					this->_forward_udp_data(wptr.lock(), endpoint, data);
				});

				this->udp_dst_port_ = std::uint16_t(std::strtoul(port.data(), nullptr, 10));

				// async start the back client
				if (!back_client->async_start(std::move(host), 0))
					return nullptr;

				// set timer expire, the socks5 async handshake will wait this timer by coroutine.
				back_client->connect_finish_timer_->expires_after(std::chrono::steady_clock::duration::max());

				this->back_client_ = back_client;

				return back_client->connect_finish_timer_.get();
			}
			else
			{
				return nullptr;
			}
		}

		// recvd data from udp
		void _forward_udp_data(
			std::shared_ptr<asio2::socks5_udp_transfer> udp_cast_ptr,
			asio::ip::udp::endpoint& endpoint, std::string_view data)
		{
			if (!udp_cast_ptr)
				return;

			ASIO2_ASSERT(udp_cast_ptr->running_in_this_thread());
			ASIO2_ASSERT(this->running_in_this_thread());

			// socks5 session is the front client.
			asio::ip::address front_addr = this->socket().lowest_layer().remote_endpoint().address();

			bool is_from_front = false;

			if (front_addr.is_loopback())
				is_from_front = (endpoint.address() == front_addr && endpoint.port() == this->udp_dst_port_);
			else
				is_from_front = (endpoint.address() == front_addr);

			// recvd data from the front client. forward it to the target endpoint.
			if (is_from_front)
			{
				// when socks5 server recvd data from the front client by udp, set the channel flag.
				this->udp_dst_channel_ = asio2::net_protocol::udp;

				auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, false);
				if (err == 0)
				{
					if (domain.empty())
						udp_cast_ptr->async_send(std::move(ep), real_data);
					else
						udp_cast_ptr->async_send(std::move(domain), ep.port(), real_data);
				}
			}
			// recvd data not from the front client. forward it to the front client.
			else
			{
				std::vector<std::uint8_t> edit_data;
				edit_data.reserve(data.size() + 4 + 4 + 2);

				edit_data.push_back(std::uint8_t(0x00)); // RSV 
				edit_data.push_back(std::uint8_t(0x00)); // RSV 
				edit_data.push_back(std::uint8_t(0x00)); // FRAG 

				asio::ip::address end_addr = endpoint.address();

				if (end_addr.is_v4()) // IP V4 address: X'01'
				{
					edit_data.push_back(std::uint8_t(0x01));
					asio::ip::address_v4::bytes_type bytes = end_addr.to_v4().to_bytes();
					edit_data.insert(edit_data.cend(), bytes.begin(), bytes.end());
				}
				else if (end_addr.is_v6()) // IP V6 address: X'04'
				{
					edit_data.push_back(std::uint8_t(0x04));
					asio::ip::address_v6::bytes_type bytes = end_addr.to_v6().to_bytes();
					edit_data.insert(edit_data.cend(), bytes.begin(), bytes.end());
				}

				std::uint16_t uport = detail::host_to_network(std::uint16_t(endpoint.port()));
				std::uint8_t* pport = reinterpret_cast<std::uint8_t*>(std::addressof(uport));
				edit_data.push_back(pport[0]);
				edit_data.push_back(pport[1]);
				edit_data.insert(edit_data.cend(), data.begin(), data.end());

				if (this->udp_dst_channel_ == asio2::net_protocol::tcp)
				{
					std::uint16_t udatalen = detail::host_to_network(std::uint16_t(data.size()));
					std::uint8_t* pdatalen = reinterpret_cast<std::uint8_t*>(std::addressof(udatalen));
					edit_data[0] = pdatalen[0];
					edit_data[1] = pdatalen[1];

					this->derived().async_send(std::move(edit_data));
				}
				else if (this->udp_dst_channel_ == asio2::net_protocol::udp)
				{
					udp_cast_ptr->async_send(
						asio::ip::udp::endpoint(front_addr, this->udp_dst_port_), std::move(edit_data));
				}
			}
		}

		template<typename C, typename DeferEvent>
		inline void _handle_connect(
			const error_code& ec,
			std::shared_ptr<derived_t> this_ptr, std::shared_ptr<ecs_t<C>> ecs, DeferEvent chain)
		{
			detail::ignore_unused(ec);

			derived_t& derive = this->derived();

			ASIO2_ASSERT(!ec);
			ASIO2_ASSERT(derive.sessions_.io_->running_in_this_thread());

			asio::dispatch(derive.io_->context(), make_allocator(derive.wallocator(),
			[this, &derive, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
			() mutable
			{
				socks5_async_handshake(derive.socket(), derive.socks5_options_,
				[&derive, this_ptr](auto& s5_server_handshake_op) mutable
				{
					return derive._handle_socks5_command(std::move(this_ptr), s5_server_handshake_op);
				},
				[this, &derive, this_ptr, ecs = std::move(ecs), chain = std::move(chain)]
				(const error_code& ec) mutable
				{
					derive.sessions_.dispatch(
					[this, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
					() mutable
					{
						set_last_error(ec);

						this->derived()._fire_socks5_handshake(this_ptr);

						super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					});
				});
			}));
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			// front client has disconnected, so we need disconnect the back client too.
			std::visit([](auto& ptr) mutable
			{
				if (ptr)
				{
					ptr->stop();
					ptr.reset();
				}
			}, this->back_client_);

			super::_handle_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>&, std::string_view data)
		{
			this->listener_.notify(event_type::recv, this_ptr, data);

			if (data.empty())
				return;

			// recvd data from the front client by tcp, we need check the front client is tcp or udp.
			std::visit(variant_overloaded
			{
				[](auto) {},
				// if it is tcp, just forward the data to back client.
				[data](std::shared_ptr<asio2::socks5_tcp_transfer>& back_client_ptr) mutable
				{
					if (back_client_ptr)
						back_client_ptr->async_send(data);
				},
				// if it is udp, it means that the packet is a extension protocol base of below:
				// +----+------+------+----------+----------+----------+
				// |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
				// +----+------+------+----------+----------+----------+
				// | 2  |  1   |  1   | Variable |    2     | Variable |
				// +----+------+------+----------+----------+----------+
				// the RSV field is the real data length of the field DATA.
				// so we need unpacket this data, and send the real data to the back client.
				[this, data](std::shared_ptr<asio2::socks5_udp_transfer>& back_client_ptr) mutable
				{
					// when socks5 server recvd data from the front client by tcp, set the channel flag.
					this->udp_dst_channel_ = asio2::net_protocol::tcp;

					if (back_client_ptr)
					{
						auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, true);
						if (err == 0)
						{
							if (domain.empty())
								back_client_ptr->async_send(std::move(ep), real_data);
							else
								back_client_ptr->async_send(std::move(domain), ep.port(), real_data);
						}
					}
				}
			}, this->back_client_);
		}

		inline void _fire_socks5_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_socks5_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

	protected:
		socks5::options      socks5_options_{};

		// if the command is UDP ASSOCIATE, we need save the port of the front udp client.
		std::uint16_t        udp_dst_port_{};

		// if the front client is udp, and when socks5 server recvd data from the back client, 
		// whether we use tcp channel or udp channcel to forward the data to the front client?
		asio2::net_protocol  udp_dst_channel_ = asio2::net_protocol::udp;

		// create a new client, and connect to the target server.
		// if the command is CONNECT, we need create a tcp client.
		// if the command is UDP ASSOCIATE, we need create a udp cast.
		std::variant<
			std::shared_ptr<asio2::socks5_tcp_transfer>,
			std::shared_ptr<asio2::socks5_udp_transfer>> back_client_;
	};
}

namespace asio2
{
	using socks5_session_args = detail::template_args_tcp_session;

	template<class derived_t, class args_t>
	using socks5_session_impl_t = detail::socks5_session_impl_t<derived_t, args_t>;

	/**
	 * @brief socks5 tcp session
	 */
	template<class derived_t>
	class socks5_session_t : public detail::socks5_session_impl_t<derived_t, detail::template_args_tcp_session>
	{
	public:
		using detail::socks5_session_impl_t<derived_t, detail::template_args_tcp_session>::socks5_session_impl_t;
	};

	/**
	 * @brief socks5 tcp session
	 */
	class socks5_session : public socks5_session_t<socks5_session>
	{
	public:
		using socks5_session_t<socks5_session>::socks5_session_t;
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_SOCKS5_SESSION_HPP__
