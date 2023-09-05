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
#include <vector>
#include <tuple>

#include <asio2/component/socks/socks5_core.hpp>

namespace asio2::socks5
{
	/**
	 * @return tuple: error, endpoint, domain, real_data.
	 */
std::tuple<int, asio::ip::udp::endpoint, std::string_view, std::string_view> parse_udp_packet(
	std::string_view data, bool need_check_data_len)
{
	// RSV FRAG 
	if (data.size() < std::size_t(3))
		return { 1,{},{},{} };

	std::uint16_t data_size = *(reinterpret_cast<const std::uint16_t*>(data.data()));

	// use little endian
	if (!asio2::detail::is_little_endian())
	{
		asio2::detail::swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(std::addressof(data_size)));
	}

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

		if (need_check_data_len && data.size() != data_size)
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

		if (need_check_data_len && data.size() != data_size)
			return { 6,{},{},{} };
	}
	else if (atyp == std::uint8_t(0x03)) // DOMAINNAME: X'03'
	{
		if (data.size() < std::size_t(1))
			return { 7,{},{},{} };

		std::uint8_t domain_len = std::uint8_t(data[0]);

		data.remove_prefix(1);

		if (data.size() < std::size_t(domain_len + 2))
			return { 8,{},{},{} };

		domain = data.substr(0, domain_len);

		data.remove_prefix(domain_len);

		auto* p = data.data();

		std::uint16_t uport = asio2::detail::read<std::uint16_t>(p);
		endpoint.port(uport);

		data.remove_prefix(2);

		if (need_check_data_len && data.size() != data_size)
			return { 9,{},{},{} };
	}
	else
	{
		return { 10,{},{},{} };
	}

	return { 0, std::move(endpoint), domain, data };
}
}

#endif // !__ASIO2_SOCKS5_UTIL_HPP__
