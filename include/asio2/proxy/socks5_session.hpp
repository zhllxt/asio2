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

				client->bind_start([]() mutable
				{
					set_last_error(get_last_error());
				}).bind_stop([session_ptr]() mutable
				{
					session_ptr->stop();
				}).bind_recv([session_ptr](asio::ip::udp::endpoint& endpoint, std::string_view data) mutable
				{
					session_ptr->async_send(data);
				});

				if (!client->async_start(std::move(host), std::move(port)))
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

			std::visit(variant_overloaded
			{
				[](auto) {},
				[data](std::shared_ptr<asio2::socks5_tcp_client>& client_ptr) mutable
				{
					if (client_ptr)
						client_ptr->async_send(data);
				},
				[this, data](std::shared_ptr<asio2::socks5_udp_client>& client_ptr) mutable
				{
					if (client_ptr)
						this->derived()._forward_udp_data(client_ptr, data);
				}
			}, this->end_client_);
		}

		inline void _forward_udp_data(std::shared_ptr<asio2::socks5_udp_client>& client_ptr, std::string_view data)
		{
			// RSV FRAG 
			if (data.size() < std::size_t(3))
				return;

			std::uint16_t data_size = *(reinterpret_cast<const std::uint16_t*>(data.data()));

			// use little endian
			if (!detail::is_little_endian())
			{
				swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(std::addressof(data_size)));
			}

			data.remove_prefix(3);

			// ATYP 
			if (data.size() < std::size_t(1))
				return;

			std::uint8_t atyp = std::uint8_t(data[0]);

			data.remove_prefix(1);

			if /**/ (atyp == std::uint8_t(0x01)) // IP V4 address: X'01'
			{
				if (data.size() < std::size_t(4 + 2))
					return;

				asio::ip::udp::endpoint endpoint{};

				asio::ip::address_v4::bytes_type addr{};
				for (std::size_t i = 0; i < addr.size(); i++)
				{
					addr[i] = asio::ip::address_v4::bytes_type::value_type(data[i]);
				}
				endpoint.address(asio::ip::address_v4(addr));

				data.remove_prefix(4);

				auto* p = data.data();

				std::uint16_t uport = detail::read<std::uint16_t>(p);
				endpoint.port(uport);

				data.remove_prefix(2);

				if (data.size() != data_size)
					return;

				client_ptr->async_send(endpoint, data);
			}
			else if (atyp == std::uint8_t(0x04)) // IP V6 address: X'04'
			{
				if (data.size() < std::size_t(16 + 2))
					return;

				data.remove_prefix(16 + 2);

				asio::ip::udp::endpoint endpoint{};

				asio::ip::address_v6::bytes_type addr{};
				for (std::size_t i = 0; i < addr.size(); i++)
				{
					addr[i] = asio::ip::address_v6::bytes_type::value_type(data[i]);
				}
				endpoint.address(asio::ip::address_v6(addr));

				data.remove_prefix(16);

				auto* p = data.data();

				std::uint16_t uport = detail::read<std::uint16_t>(p);
				endpoint.port(uport);

				data.remove_prefix(2);

				if (data.size() != data_size)
					return;

				client_ptr->async_send(endpoint, data);
			}
			else if (atyp == std::uint8_t(0x03)) // DOMAINNAME: X'03'
			{
				if (data.size() < std::size_t(1))
					return;

				std::uint8_t domain_len = std::uint8_t(data[0]);

				data.remove_prefix(1);

				if (data.size() < std::size_t(domain_len + 2))
					return;

				std::string domain{ data.substr(0, domain_len) };

				data.remove_prefix(domain_len);

				auto* p = data.data();

				std::uint16_t uport = detail::read<std::uint16_t>(p);

				data.remove_prefix(2);

				if (data.size() != data_size)
					return;

				client_ptr->async_send(std::move(domain), uport, data);
			}
			else
			{
				return;
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
