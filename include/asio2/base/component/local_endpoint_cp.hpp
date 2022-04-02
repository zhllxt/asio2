/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_LOCAL_ENDPOINT_COMPONENT_HPP__
#define __ASIO2_LOCAL_ENDPOINT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/3rd/asio.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class local_endpoint_cp
	{
	public:
		using endpoint_type = typename args_t::socket_t::lowest_layer_type::endpoint_type;
		using port_type     = std::uint16_t; // typename asio::ip::port_type; // boost 1.72 not exists this type.

		/**
		 * @constructor
		 */
		local_endpoint_cp() noexcept : local_endpoint_() {}

		/**
		 * @destructor
		 */
		~local_endpoint_cp() = default;

		local_endpoint_cp(local_endpoint_cp&&) noexcept = default;
		local_endpoint_cp(local_endpoint_cp const&) noexcept = default;
		local_endpoint_cp& operator=(local_endpoint_cp&&) noexcept = default;
		local_endpoint_cp& operator=(local_endpoint_cp const&) noexcept = default;

	public:
		/**
		 * Construct an endpoint using a port number, specified in the host's byte
		 * order. The IP address will be the any address (i.e. INADDR_ANY or
		 * in6addr_any).
		 * To initialise an IPv4 TCP endpoint for port 1234, use:
		 * asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 1234);
		 * To specify an IPv6 UDP endpoint for port 9876, use:
		 * asio::ip::udp::endpoint ep(asio::ip::udp::v6(), 9876);
		 * same as set_local_endpoint
		 */
		template<typename InternetProtocol, typename StrOrInt>
		inline derived_t& local_endpoint(const InternetProtocol& protocol, StrOrInt&& port)
		{
			return this->set_local_endpoint(protocol, std::forward<StrOrInt>(port));
		}

		/**
		 * Construct an endpoint using a port number and an IP address. This
		 * function may be used for accepting connections on a specific interface
		 * or for making a connection to a remote endpoint.
		 * To initialise an IPv4 TCP endpoint for port 1234, use:
		 * asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string("..."), 1234);
		 * To specify an IPv6 UDP endpoint for port 9876, use:
		 * asio::ip::udp::endpoint ep(asio::ip::address_v6::from_string("..."), 9876);
		 * same as set_local_endpoint
		 */
		template<typename StrOrInt>
		inline derived_t& local_endpoint(const asio::ip::address& addr, StrOrInt&& port)
		{
			return this->set_local_endpoint(addr, std::forward<StrOrInt>(port));
		}

		/**
		 * Construct an endpoint using a port number, specified in the host's byte
		 * order. The IP address will be the any address (i.e. INADDR_ANY or
		 * in6addr_any).
		 * To initialise an IPv4 TCP endpoint for port 1234, use:
		 * asio::ip::tcp::endpoint ep(asio::ip::tcp::v4(), 1234);
		 * To specify an IPv6 UDP endpoint for port 9876, use:
		 * asio::ip::udp::endpoint ep(asio::ip::udp::v6(), 9876);
		 */
		template<typename InternetProtocol, typename StrOrInt>
		inline derived_t& set_local_endpoint(const InternetProtocol& protocol, StrOrInt&& port)
		{
			this->local_endpoint_ = endpoint_type(protocol,
				detail::to_integer<port_type>(std::forward<StrOrInt>(port)));
			return static_cast<derived_t&>(*this);
		}

		/**
		 * Construct an endpoint using a port number and an IP address. This
		 * function may be used for accepting connections on a specific interface
		 * or for making a connection to a remote endpoint.
		 * To initialise an IPv4 TCP endpoint for port 1234, use:
		 * asio::ip::tcp::endpoint ep(asio::ip::address_v4::from_string("..."), 1234);
		 * To specify an IPv6 UDP endpoint for port 9876, use:
		 * asio::ip::udp::endpoint ep(asio::ip::address_v6::from_string("..."), 9876);
		 */
		template<typename StrOrInt>
		inline derived_t& set_local_endpoint(const asio::ip::address& addr, StrOrInt&& port)
		{
			this->local_endpoint_ = endpoint_type(addr,
				detail::to_integer<port_type>(std::forward<StrOrInt>(port)));
			return static_cast<derived_t&>(*this);
		}

		/**
		 * get the binded local endpoint refrence, same as get_local_endpoint
		 */
		inline endpoint_type& local_endpoint() noexcept { return this->get_local_endpoint(); }

		/**
		 * get the binded local endpoint refrence
		 */
		inline endpoint_type& get_local_endpoint() noexcept { return this->local_endpoint_; }

	protected:
		/// local bind endpoint
		endpoint_type local_endpoint_;
	};
}

#endif // !__ASIO2_LOCAL_ENDPOINT_COMPONENT_HPP__
