//
// ipv4_header.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ASIO2_IPV4_HEADER_HPP__
#define __ASIO2_IPV4_HEADER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>

#include <asio2/external/asio.hpp>

// Packet header for IPv4.
//
// The wire format of an IPv4 header is:
// 
// 0               8               16                             31
// +-------+-------+---------------+------------------------------+      ---
// |       |       |               |                              |       ^
// |version|header |    type of    |    total length in bytes     |       |
// |  (4)  | length|    service    |                              |       |
// +-------+-------+---------------+-+-+-+------------------------+       |
// |                               | | | |                        |       |
// |        identification         |0|D|M|    fragment offset     |       |
// |                               | |F|F|                        |       |
// +---------------+---------------+-+-+-+------------------------+       |
// |               |               |                              |       |
// | time to live  |   protocol    |       header checksum        |   20 bytes
// |               |               |                              |       |
// +---------------+---------------+------------------------------+       |
// |                                                              |       |
// |                      source IPv4 address                     |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination IPv4 address                   |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
// |                                                              |       ^
// |                                                              |       |
// /                        options (if any)                      /    0 - 40
// /                                                              /     bytes
// |                                                              |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---


namespace asio2::detail
{
	class ipv4_header
	{
	public:
		ipv4_header()
		{ 
			std::fill(rep_, rep_ + sizeof(rep_), static_cast<unsigned char>(0));
		}

		inline unsigned char  version()         const { return (rep_[0] >> 4) & 0xF; }
		inline unsigned short header_length()   const { return (unsigned short)((rep_[0] & 0xF) * 4); }
		inline unsigned char  type_of_service() const { return rep_[1]; }
		inline unsigned short total_length()    const { return decode(2, 3); }
		inline unsigned short identification()  const { return decode(4, 5); }
		inline bool           dont_fragment()   const { return (rep_[6] & 0x40) != 0; }
		inline bool           more_fragments()  const { return (rep_[6] & 0x20) != 0; }
		inline unsigned short fragment_offset() const { return decode(6, 7) & 0x1FFF; }
		inline unsigned int   time_to_live()    const { return rep_[8]; }
		inline unsigned char  protocol()        const { return rep_[9]; }
		inline unsigned short header_checksum() const { return decode(10, 11); }

		inline asio::ip::address_v4 source_address() const
		{
			asio::ip::address_v4::bytes_type bytes = { { rep_[12], rep_[13], rep_[14], rep_[15] } };
			return asio::ip::address_v4(bytes);
		}

		inline asio::ip::address_v4 destination_address() const
		{
			asio::ip::address_v4::bytes_type bytes = { { rep_[16], rep_[17], rep_[18], rep_[19] } };
			return asio::ip::address_v4(bytes);
		}

		friend std::istream& operator>>(std::istream& is, ipv4_header& header)
		{
			is.read(reinterpret_cast<char*>(header.rep_), 20);
			if (header.version() != 4)
				is.setstate(std::ios::failbit);
			std::streamsize options_length = header.header_length() - 20;
			if (options_length < 0 || options_length > 40)
				is.setstate(std::ios::failbit);
			else
				is.read(reinterpret_cast<char*>(header.rep_) + 20, options_length);
			return is;
		}

	private:
		inline unsigned short decode(int a, int b) const
		{
			return (unsigned short)((rep_[a] << 8) + rep_[b]);
		}

		unsigned char rep_[60];
	};
}

#endif // __ASIO2_IPV4_HEADER_HPP__
