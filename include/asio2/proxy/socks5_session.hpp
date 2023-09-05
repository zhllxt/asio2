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
#include <asio2/proxy/socks5_client.hpp>

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
			}, this->end_client_);

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
			switch (this->end_client_.index())
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
			auto host = s5_server_handshake_op.host;
			auto port = s5_server_handshake_op.port;

			if (s5_server_handshake_op.cmd == socks5::command::connect)
			{
				auto client = std::make_shared<asio2::socks5_tcp_client>(this->io_);

				client->bind_connect([]() mutable
				{
					set_last_error(get_last_error());
				}).bind_disconnect([session_ptr]() mutable
				{
					session_ptr->stop();
				}).bind_recv([session_ptr](std::string_view data) mutable
				{
					session_ptr->async_send(data);
				});

				if (!client->async_start(std::move(host), std::move(port)))
					return nullptr;

				client->connect_finish_timer_->expires_after(std::chrono::steady_clock::duration::max());

				this->end_client_ = client;

				return client->connect_finish_timer_.get();
			}
			else if (s5_server_handshake_op.cmd == socks5::command::udp_associate)
			{
				auto client = std::make_shared<asio2::socks5_udp_client>(this->io_);

				client->bind_start([strbuf = s5_server_handshake_op.stream.get(), pclient = client.get()]() mutable
				{
					set_last_error(get_last_error());

					std::uint16_t uport = pclient->get_local_port();

					char* p = const_cast<char*>(static_cast<const char*>(strbuf->data().data()));

					std::uint8_t addr_type = std::uint8_t(p[3]);

					if /**/ (addr_type == std::uint8_t(0x01))
					{
						p += 1 + 1 + 1 + 1 + 4;

						detail::write(p, uport);
					}
					else if (addr_type == std::uint8_t(0x04))
					{
						p += 1 + 1 + 1 + 1 + 16;

						detail::write(p, uport);
					}
					else if (addr_type == std::uint8_t(0x03))
					{
						// real length
						p += 1 + 1 + 1 + 1 + 1 + p[4];

						detail::write(p, uport);
					}
				}).bind_stop([session_ptr]() mutable
				{
					session_ptr->stop();
				}).bind_recv([this, pclient = client.get()](asio::ip::udp::endpoint& endpoint, std::string_view data) mutable
				{
					// recvd data, maybe from front client or end client, we need to check whether from front or end by ip.
					this->_forward_udp_data(pclient, endpoint, data);
				});

				this->udp_associate_port_ = std::uint16_t(std::strtoul(port.data(), nullptr, 10));

				if (!client->async_start(std::move(host), 0))
					return nullptr;

				client->connect_finish_timer_->expires_after(std::chrono::steady_clock::duration::max());

				this->end_client_ = client;

				return client->connect_finish_timer_.get();
			}
			else
			{
				return nullptr;
			}
		}

		// recvd data from udp
		template<class T>
		void _forward_udp_data(T* client, asio::ip::udp::endpoint& endpoint, std::string_view data)
		{
			// socks5 session is the front client.
			asio::ip::address front_addr = this->socket().lowest_layer().remote_endpoint().address();

			bool is_from_front = false;

			if (front_addr.is_loopback())
				is_from_front = (endpoint.address() == front_addr && endpoint.port() == this->udp_associate_port_);
			else
				is_from_front = (endpoint.address() == front_addr);

			// recvd data from the front client. forward it to the target endpoint.
			if (is_from_front)
			{
				auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, false);
				if (err == 0)
				{
					if (domain.empty())
						client->async_send(std::move(ep), real_data);
					else
						client->async_send(std::move(domain), ep.port(), real_data);
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
					edit_data.insert(edit_data.end(), bytes.begin(), bytes.end());
				}
				else if (end_addr.is_v6()) // IP V6 address: X'04'
				{
					edit_data.push_back(std::uint8_t(0x04));
					asio::ip::address_v6::bytes_type bytes = end_addr.to_v6().to_bytes();
					edit_data.insert(edit_data.end(), bytes.begin(), bytes.end());
				}

				std::uint16_t uport = endpoint.port();
				std::uint8_t* pport = reinterpret_cast<std::uint8_t*>(std::addressof(uport));
				if (detail::is_little_endian())
				{
					swap_bytes<sizeof(std::uint16_t)>(pport);
				}
				edit_data.push_back(pport[0]);
				edit_data.push_back(pport[1]);
				edit_data.insert(edit_data.end(), data.begin(), data.end());

				client->async_send(asio::ip::udp::endpoint(front_addr, this->udp_associate_port_), std::move(edit_data));
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
					[this, &derive, ec, this_ptr = std::move(this_ptr), ecs = std::move(ecs), chain = std::move(chain)]
					() mutable
					{
						set_last_error(ec);

						derive._fire_socks5_handshake(this_ptr);

						super::_handle_connect(ec, std::move(this_ptr), std::move(ecs), std::move(chain));
					});
				});
			}));
		}

		template<typename DeferEvent>
		inline void _handle_stop(const error_code& ec, std::shared_ptr<derived_t> this_ptr, DeferEvent chain)
		{
			std::visit([](auto& ptr) mutable
			{
				if (ptr)
				{
					ptr->stop();
					ptr.reset();
				}
			}, this->end_client_);

			super::_handle_stop(ec, std::move(this_ptr), std::move(chain));
		}

		template<typename C>
		inline void _fire_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>&, std::string_view data)
		{
			this->listener_.notify(event_type::recv, this_ptr, data);

			if (data.empty())
				return;

			// recvd data from the front client by tcp, forward the data to the end client.
			std::visit(variant_overloaded
			{
				[](auto) {},
				[data](std::shared_ptr<asio2::socks5_tcp_client>& end_client_ptr) mutable
				{
					if (end_client_ptr)
						end_client_ptr->async_send(data);
				},
				[this, data](std::shared_ptr<asio2::socks5_udp_client>& end_client_ptr) mutable
				{
					if (end_client_ptr)
						this->derived()._forward_udp_data(end_client_ptr, data);
				}
			}, this->end_client_);
		}

		// recvd data from tcp
		inline void _forward_udp_data(std::shared_ptr<asio2::socks5_udp_client>& end_client_ptr, std::string_view data)
		{
			auto [err, ep, domain, real_data] = asio2::socks5::parse_udp_packet(data, true);
			if (err == 0)
			{
				if (domain.empty())
					end_client_ptr->async_send(std::move(ep), real_data);
				else
					end_client_ptr->async_send(std::move(domain), ep.port(), real_data);
			}
		}

		inline void _fire_socks5_handshake(std::shared_ptr<derived_t>& this_ptr)
		{
			// the _fire_socks5_handshake must be executed in the thread 0.
			ASIO2_ASSERT(this->sessions_.io_->running_in_this_thread());

			this->listener_.notify(event_type::upgrade, this_ptr);
		}

	protected:
		socks5::options socks5_options_{};

		std::uint16_t   udp_associate_port_{};

		// create a new client, and connect to the target server.
		std::variant<
			std::shared_ptr<asio2::socks5_tcp_client>,
			std::shared_ptr<asio2::socks5_udp_client>> end_client_;
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
