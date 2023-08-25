/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKET_COMPONENT_HPP__
#define __ASIO2_SOCKET_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>

#include <asio2/external/asio.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class socket_cp
	{
	public:
		using socket_type = std::remove_cv_t<std::remove_reference_t<typename args_t::socket_t>>;

		/**
		 * @brief constructor
		 * @throws maybe throw exception "Too many open files" (exception code : 24)
		 * asio::error::no_descriptors - Too many open files
		 */
		explicit socket_cp(asio::io_context& ioc) : socket_(std::make_shared<socket_type>(ioc))
		{
		}

		/**
		 * @brief constructor
		 * @li for udp session
		 */
		explicit socket_cp(std::shared_ptr<typename args_t::socket_t> ptr) : socket_(std::move(ptr))
		{
		}

		/**
		 * @brief destructor
		 */
		~socket_cp()
		{
		}

	public:
		/**
		 * @brief get the socket object reference
		 */
		inline socket_type & socket() noexcept
		{
			return *(this->socket_);
		}

		/**
		 * @brief get the socket object reference
		 */
		inline const socket_type & socket() const noexcept
		{
			return *(this->socket_);
		}

		/**
		 * @brief get the stream object reference
		 */
		inline socket_type & stream() noexcept
		{
			return *(this->socket_);
		}

		/**
		 * @brief get the stream object reference
		 */
		inline const socket_type & stream() const noexcept
		{
			return *(this->socket_);
		}

		/**
		 * @brief get the local address, same as get_local_address
		 */
		inline std::string local_address() const noexcept
		{
			return this->get_local_address();
		}

		/**
		 * @brief get the local address
		 */
		inline std::string get_local_address() const noexcept
		{
			clear_last_error();
			try
			{
				return this->socket_->lowest_layer().local_endpoint().address().to_string();
			}
			catch (const system_error& e)
			{
				set_last_error(e);
			}
			return std::string();
		}

		/**
		 * @brief get the local port, same as get_local_port
		 */
		inline unsigned short local_port() const noexcept
		{
			return this->get_local_port();
		}

		/**
		 * @brief get the local port
		 */
		inline unsigned short get_local_port() const noexcept
		{
			return this->socket_->lowest_layer().local_endpoint(get_last_error()).port();
		}

		/**
		 * @brief get the remote address, same as get_remote_address
		 */
		inline std::string remote_address() const noexcept
		{
			return this->get_remote_address();
		}

		/**
		 * @brief get the remote address
		 */
		inline std::string get_remote_address() const noexcept
		{
			clear_last_error();

			error_code ec{};

			try
			{
				return this->socket_->lowest_layer().remote_endpoint().address().to_string();
			}
			catch (const system_error& e)
			{
				ec = e.code();
			}

			try
			{
				asio::ip::address addr = this->remote_endpoint_.address();

				if (!addr.is_unspecified())
				{
					return addr.to_string();
				}
			}
			catch (const system_error&)
			{
			}

			set_last_error(ec);

			return std::string();
		}

		/**
		 * @brief get the remote port, same as get_remote_port
		 */
		inline unsigned short remote_port() const noexcept
		{
			return this->get_remote_port();
		}

		/**
		 * @brief get the remote port
		 */
		inline unsigned short get_remote_port() const noexcept
		{
			clear_last_error();

			error_code ec{};

			try
			{
				return this->socket_->lowest_layer().remote_endpoint().port();
			}
			catch (const system_error& e)
			{
				ec = e.code();
			}

			try
			{
				return this->remote_endpoint_.port();
			}
			catch (const system_error&)
			{
			}

			set_last_error(ec);

			return 0;
		}

	public:
		/**
		 * @brief Implements the SOL_SOCKET/SO_SNDBUF socket option.
		 */
		inline derived_t& set_sndbuf_size(int val) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::socket_base::send_buffer_size(val), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_SNDBUF socket option.
		 */
		inline int get_sndbuf_size() const noexcept
		{
			asio::socket_base::send_buffer_size option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option.value();
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_RCVBUF socket option.
		 */
		inline derived_t& set_rcvbuf_size(int val) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::socket_base::receive_buffer_size(val), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_RCVBUF socket option.
		 */
		inline int get_rcvbuf_size() const noexcept
		{
			asio::socket_base::receive_buffer_size option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option.value();
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_KEEPALIVE socket option. same as set_keep_alive
		 */
		inline derived_t & keep_alive(bool val) noexcept
		{
			return this->set_keep_alive(val);
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_KEEPALIVE socket option.
		 */
		inline derived_t& set_keep_alive(bool val) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::socket_base::keep_alive(val), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_KEEPALIVE socket option.
		 */
		inline bool is_keep_alive() const noexcept
		{
			asio::socket_base::keep_alive option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option.value();
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_REUSEADDR socket option. same as set_reuse_address
		 */
		inline derived_t & reuse_address(bool val) noexcept
		{
			return this->set_reuse_address(val);
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_REUSEADDR socket option.
		 */
		inline derived_t& set_reuse_address(bool val) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::socket_base::reuse_address(val), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Implements the SOL_SOCKET/SO_REUSEADDR socket option.
		 */
		inline bool is_reuse_address() const noexcept
		{
			asio::socket_base::reuse_address option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option.value();
		}

		/**
		 * @brief Implements the TCP_NODELAY socket option. same as set_no_delay.
		 * If it's not a tcp socket, do nothing
		 */
		inline derived_t & no_delay(bool val) noexcept
		{
			return this->set_no_delay(val);
		}

		/**
		 * @brief Implements the TCP_NODELAY socket option.
		 * If it's not a tcp socket, do nothing
		 */
		inline derived_t& set_no_delay(bool val) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::ip::tcp::no_delay(val), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Implements the TCP_NODELAY socket option.
		 */
		inline bool is_no_delay() const noexcept
		{
			asio::ip::tcp::no_delay option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option.value();
		}

		/**
		 * @brief Implements the SO_LINGER socket option.
		 *        set_linger(true, 0) - RST will be sent instead of FIN/ACK/FIN/ACK
		 * @param enable - option on/off
		 * @param timeout - linger time
		 */
		inline derived_t& set_linger(bool enable, int timeout) noexcept
		{
			this->socket_->lowest_layer().set_option(asio::socket_base::linger(enable, timeout), get_last_error());
			return (static_cast<derived_t&>(*this));
		}

		/**
		 * @brief Get the SO_LINGER socket option.
		 */
		inline asio::socket_base::linger get_linger() const noexcept
		{
			asio::socket_base::linger option{};
			this->socket_->lowest_layer().get_option(option, get_last_error());
			return option;
		}

	protected:
		/// socket 
		/// 20230802 change this member variable from "typename args_t::socket_t socket_;"
		/// to "std::shared_ptr<typename args_t::socket_t> socket_;", why? beacuse the 
		/// socket shoule be destroyed before the io_context destroyed, otherwise maybe 
		/// cause crash, so we use a shared_ptr to manually manage the lifecycle of socket.
		std::shared_ptr<typename args_t::socket_t> socket_;

		/// the call of remote_endpoint() maybe failed when the remote socket is closed, 
		/// even if local socket is not closed, so we use this variable to ensure the
		/// call of remote_endpoint() must be successed.
		typename socket_type::endpoint_type remote_endpoint_{};
	};
}

#endif // !__ASIO2_SOCKET_COMPONENT_HPP__
