/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_SOCKET_COMPONENT_HPP__
#define __ASIO2_SOCKET_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>

#include <asio2/base/selector.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class socket_cp
	{
	public:
		using socket_type = std::remove_cv_t<std::remove_reference_t<typename args_t::socket_t>>;

		/**
		 * @constructor
		 */
		template<class ...Args>
		explicit socket_cp(Args&&... args) : socket_(std::forward<Args>(args)...)
		{
		}

		/**
		 * @destructor
		 */
		~socket_cp() = default;

	public:
		/**
		 * @function : get the socket object refrence
		 */
		inline socket_type & socket()
		{
			return this->socket_;
		}

		/**
		 * @function : get the stream object refrence
		 */
		inline socket_type & stream()
		{
			return this->socket_;
		}

		/**
		 * @function : get the local address
		 */
		inline std::string local_address()
		{
			try
			{
				return this->socket_.lowest_layer().local_endpoint().address().to_string();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return std::string();
		}

		/**
		 * @function : get the local port
		 */
		inline unsigned short local_port()
		{
			try
			{
				return this->socket_.lowest_layer().local_endpoint().port();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return static_cast<unsigned short>(0);
		}

		/**
		 * @function : get the remote address
		 */
		inline std::string remote_address()
		{
			try
			{
				return this->socket_.lowest_layer().remote_endpoint().address().to_string();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return std::string();
		}

		/**
		 * @function : get the remote port
		 */
		inline unsigned short remote_port()
		{
			try
			{
				return this->socket_.lowest_layer().remote_endpoint().port();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return static_cast<unsigned short>(0);
		}

	public:
		/**
		 * @function : Implements the SOL_SOCKET/SO_SNDBUF socket option.
		 */
		inline derived_t & sndbuf_size(int val)
		{
			try
			{
				this->socket_.lowest_layer().set_option(asio::socket_base::send_buffer_size(val));
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_RCVBUF socket option.
		 */
		inline derived_t & rcvbuf_size(int val)
		{
			try
			{
				this->socket_.lowest_layer().set_option(asio::socket_base::receive_buffer_size(val));
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_KEEPALIVE socket option.
		 */
		inline derived_t & keep_alive(bool val)
		{
			try
			{
				this->socket_.lowest_layer().set_option(asio::socket_base::keep_alive(val));
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_REUSEADDR socket option.
		 */
		inline derived_t & reuse_address(bool val)
		{
			try
			{
				this->socket_.lowest_layer().set_option(asio::socket_base::reuse_address(val));
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_SNDBUF socket option.
		 */
		inline int sndbuf_size() const
		{
			try
			{
				asio::socket_base::send_buffer_size option;
				this->socket_.lowest_layer().get_option(option);
				return option.value();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (-1);
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_RCVBUF socket option.
		 */
		inline int rcvbuf_size() const
		{
			try
			{
				asio::socket_base::receive_buffer_size option;
				this->socket_.lowest_layer().get_option(option);
				return option.value();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (-1);
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_KEEPALIVE socket option.
		 */
		inline bool keep_alive() const
		{
			try
			{
				asio::socket_base::keep_alive option;
				this->socket_.lowest_layer().get_option(option);
				return option.value();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : Implements the SOL_SOCKET/SO_REUSEADDR socket option.
		 */
		inline bool reuse_address() const
		{
			try
			{
				asio::socket_base::reuse_address option;
				this->socket_.lowest_layer().get_option(option);
				return option.value();
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : Implements the TCP_NODELAY socket option.
		 */
		inline bool no_delay() const
		{
			try
			{
				if constexpr (std::is_same_v<typename socket_type::protocol_type, asio::ip::tcp>)
				{
					asio::ip::tcp::no_delay option;
					this->socket_.lowest_layer().get_option(option);
					return option.value();
				}
				else
				{
					std::ignore = true;
					//static_assert(false, "Only tcp socket has the no_delay option");
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return false;
		}

		/**
		 * @function : Implements the TCP_NODELAY socket option.
		 * If it's not a tcp socket, do nothing
		 */
		inline derived_t & no_delay(bool val)
		{
			try
			{
				if constexpr (std::is_same_v<typename socket_type::protocol_type, asio::ip::tcp>)
				{
					this->socket_.lowest_layer().set_option(asio::ip::tcp::no_delay(val));
				}
				else
				{
					std::ignore = true;
					//static_assert(false, "Only tcp socket has the no_delay option");
				}
			}
			catch (system_error & e)
			{
				set_last_error(e);
			}
			return (static_cast<derived_t &>(*this));
		}

	protected:
		/// socket 
		typename args_t::socket_t socket_;
	};
}

#endif // !__ASIO2_SOCKET_COMPONENT_HPP__
