/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SOCKS5_UTIL_HPP__
#define __ASIO2_SOCKS5_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string_view>
#include <tuple>

#include <asio2/component/socks/socks5_core.hpp>

namespace asio2::socks5
{
namespace
{
/**
 * @return tuple: error, endpoint, domain, real_data.
 */
std::tuple<int, asio::ip::udp::endpoint, std::string_view, std::string_view>
	parse_udp_packet(std::string_view data, bool is_ext_protocol)
{
	// RSV FRAG 
	if (data.size() < std::size_t(3))
		return { 1,{},{},{} };

	std::uint16_t data_size = asio2::detail::network_to_host(
		std::uint16_t(*(reinterpret_cast<const std::uint16_t*>(data.data()))));

	data.remove_prefix(3);

	// ATYP 
	if (data.size() < std::size_t(1))
		return { 2,{},{},{} };

	std::uint8_t atyp = std::uint8_t(data[0]);

	data.remove_prefix(1);

	asio::ip::udp::endpoint endpoint{};
	std::string_view domain;

	if /**/ (atyp == std::uint8_t(0x01)) // IP V4 address: X'01'
	{
		if (data.size() < std::size_t(4 + 2))
			return { 3,{},{},{} };

		asio::ip::address_v4::bytes_type addr{};
		for (std::size_t i = 0; i < addr.size(); i++)
		{
			addr[i] = asio::ip::address_v4::bytes_type::value_type(data[i]);
		}
		endpoint.address(asio::ip::address_v4(addr));

		data.remove_prefix(4);

		auto* p = data.data();

		std::uint16_t uport = asio2::detail::read<std::uint16_t>(p);
		endpoint.port(uport);

		data.remove_prefix(2);

		if (is_ext_protocol && data.size() != data_size)
			return { 4,{},{},{} };
	}
	else if (atyp == std::uint8_t(0x04)) // IP V6 address: X'04'
	{
		if (data.size() < std::size_t(16 + 2))
			return { 5,{},{},{} };

		data.remove_prefix(16 + 2);

		asio::ip::address_v6::bytes_type addr{};
		for (std::size_t i = 0; i < addr.size(); i++)
		{
			addr[i] = asio::ip::address_v6::bytes_type::value_type(data[i]);
		}
		endpoint.address(asio::ip::address_v6(addr));

		data.remove_prefix(16);

		auto* p = data.data();

		std::uint16_t uport = asio2::detail::read<std::uint16_t>(p);
		endpoint.port(uport);

		data.remove_prefix(2);

		if (is_ext_protocol && data.size() != data_size)
			return { 6,{},{},{} };
	}
	else if (atyp == std::uint8_t(0x03)) // DOMAINNAME: X'03'
	{
		if (data.size() < std::size_t(1))
			return { 7,{},{},{} };

		std::uint8_t domain_len = std::uint8_t(data[0]);
		if (domain_len == 0)
			return { 8,{},{},{} };

		data.remove_prefix(1);

		if (data.size() < std::size_t(domain_len + 2))
			return { 9,{},{},{} };

		domain = data.substr(0, domain_len);

		data.remove_prefix(domain_len);

		auto* p = data.data();

		std::uint16_t uport = asio2::detail::read<std::uint16_t>(p);
		endpoint.port(uport);

		data.remove_prefix(2);

		if (is_ext_protocol && data.size() != data_size)
			return { 10,{},{},{} };
	}
	else
	{
		return { 11,{},{},{} };
	}

	return { 0, std::move(endpoint), domain, data };
}
}
}

namespace asio2::detail
{
namespace
{
	using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
	using diff_type = typename iterator::difference_type;

	std::pair<iterator, bool> socks5_udp_match_role(iterator begin, iterator end) noexcept
	{
		// +----+------+------+----------+----------+----------+
		// |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
		// +----+------+------+----------+----------+----------+
		// | 2  |  1   |  1   | Variable |    2     | Variable |
		// +----+------+------+----------+----------+----------+

		for (iterator p = begin; p < end;)
		{
			if (end - p < static_cast<diff_type>(5))
				break;

			std::uint16_t data_size = asio2::detail::network_to_host(
				std::uint16_t(*(reinterpret_cast<const std::uint16_t*>(p.operator->()))));

			std::uint8_t atyp = std::uint8_t(p[3]);

			diff_type need = 0;

			// ATYP
			if /**/ (atyp == std::uint8_t(0x01))
				need = static_cast<diff_type>(2 + 1 + 1 + 4 + 2 + data_size);
			else if (atyp == std::uint8_t(0x03))
				need = static_cast<diff_type>(2 + 1 + 1 + p[4] + 2 + data_size);
			else if (atyp == std::uint8_t(0x04))
				need = static_cast<diff_type>(2 + 1 + 1 + 16 + 2 + data_size);
			else
				return std::pair(begin, true);

			if (end - p < need)
				break;

			return std::pair(p + need, true);
		}
		return std::pair(begin, false);
	}
}

// on vs2017 the std::function<asio2::net_protocol()> will cause compile error.
#if defined(_MSC_VER) && (_MSC_VER < 1929)
struct socks5_server_match_role
{
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		// +----+------+------+----------+----------+----------+
		// |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
		// +----+------+------+----------+----------+----------+
		// | 2  |  1   |  1   | Variable |    2     | Variable |
		// +----+------+------+----------+----------+----------+

		for (Iterator p = begin; p < end;)
		{
			if (this->get_front_client_type && (*(this->get_front_client_type))() == asio2::net_protocol::tcp)
				return std::pair(end, true);

			return socks5_udp_match_role(p, end);
		}

		return std::pair(begin, false);
	}

	template<class T>
	void init(std::shared_ptr<T>& session_ptr)
	{
		this->get_front_client_type = std::make_shared<std::function<asio2::net_protocol()>>(
		[p = session_ptr.get()]() mutable
		{
			return p->get_front_client_type();
		});
	}

	std::shared_ptr<std::function<asio2::net_protocol()>> get_front_client_type;
};
#else
struct socks5_server_match_role
{
	template <typename Iterator>
	std::pair<Iterator, bool> operator()(Iterator begin, Iterator end) const
	{
		// +----+------+------+----------+----------+----------+
		// |RSV | FRAG | ATYP | DST.ADDR | DST.PORT |   DATA   |
		// +----+------+------+----------+----------+----------+
		// | 2  |  1   |  1   | Variable |    2     | Variable |
		// +----+------+------+----------+----------+----------+

		for (Iterator p = begin; p < end;)
		{
			if (this->get_front_client_type() == asio2::net_protocol::tcp)
				return std::pair(end, true);

			return socks5_udp_match_role(p, end);
		}

		return std::pair(begin, false);
	}

	template<class T>
	void init(std::shared_ptr<T>& session_ptr)
	{
		this->get_front_client_type = [p = session_ptr.get()]() mutable
		{
			return p->get_front_client_type();
		};
	}

	std::function<asio2::net_protocol()> get_front_client_type;
};
#endif
}

#ifdef ASIO_STANDALONE
namespace asio
#else
namespace boost::asio
#endif
{
	template <> struct is_match_condition<asio2::detail::socks5_server_match_role> : public std::true_type {};
}

#endif // !__ASIO2_SOCKS5_UTIL_HPP__
