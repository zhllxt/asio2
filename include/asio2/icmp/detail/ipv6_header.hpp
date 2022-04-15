//
// ipv6_header.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2017 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef __ASIO2_IPV6_HEADER_HPP__
#define __ASIO2_IPV6_HEADER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <algorithm>

#include <asio2/external/asio.hpp>

// Packet header for IPv6.
//
// The wire format of an IPv6 header is:
// 
// The 'ECN' is 2 bits
// 
// +-------+---------+---+---------+------------------------------+      ---
// |       |         | E |                                        |       ^
// |version|   DS    | C |               flow label               |       |
// |  (4)  |   (6)   | N |                  (20)                  |       |
// +-------+---------+---+---------+---------------+--------------+       |
// |                               |               |              |       |
// |        payload length         |  next header  |  hop limit   |       |
// |            (16)               |      (8)      |     (8)      |       |
// +---------------+---------------+------------------------------+    40 bytes
// |                                                              |       |
// |                      source IPv6 address                     |       |
// |                            (128)                             |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination IPv6 address                   |       |
// |                            (128)                             |       v
// +--------------------------------------------------------------+      ---


namespace asio2::detail
{
	class ipv6_header
	{
	public:
		ipv6_header()
		{ 
			std::fill(rep_, rep_ + sizeof(rep_), static_cast<unsigned char>(0));
		}

		inline unsigned char  version()         const { return (rep_[0] >> 4) & 0xF; }
		inline unsigned short identification()  const { return decode(4, 5); }
		inline unsigned char  next_header()     const { return rep_[6]; }
		inline unsigned char  hop_limit()       const { return rep_[7]; }

		inline asio::ip::address_v6 source_address() const
		{
			asio::ip::address_v6::bytes_type bytes =
			{
				{
					rep_[8],  rep_[9],  rep_[10], rep_[11],
					rep_[12], rep_[13], rep_[14], rep_[15],
					rep_[16], rep_[17], rep_[18], rep_[19],
					rep_[20], rep_[21], rep_[22], rep_[23]
				}
			};
			return asio::ip::address_v6(bytes);
		}

		inline asio::ip::address_v6 destination_address() const
		{
			asio::ip::address_v6::bytes_type bytes =
			{
				{
					rep_[24], rep_[25], rep_[26], rep_[27],
					rep_[28], rep_[29], rep_[30], rep_[31],
					rep_[32], rep_[33], rep_[34], rep_[35],
					rep_[36], rep_[37], rep_[38], rep_[39]
				}
			};
			return asio::ip::address_v6(bytes);
		}

		friend std::istream& operator>>(std::istream& is, ipv6_header& header)
		{
			is.read(reinterpret_cast<char*>(header.rep_), 40);
			if (header.version() != 6)
				is.setstate(std::ios::failbit);
			return is;
		}

	private:
		inline unsigned short decode(int a, int b) const
		{
			return (unsigned short)((rep_[a] << 8) + rep_[b]);
		}

		//struct IPv6hdr {
		//	unsigned int ver : 4;
		//	unsigned int traf_class : 8;
		//	unsigned int flow_lab : 20;
		//	unsigned int length : 16;
		//	unsigned int next_header : 8;
		//	unsigned int hop_limit : 8;
		//	unsigned char src_addr[16];
		//	unsigned char dest_addr[16];
		//};

		unsigned char rep_[40];
	};
}

#endif // __ASIO2_IPV6_HEADER_HPP__
