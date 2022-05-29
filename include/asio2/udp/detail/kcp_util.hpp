/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * KCP MTU , UDP PACKET MAX LENGTH 576
 */

#ifndef __ASIO2_KCP_UTIL_HPP__
#define __ASIO2_KCP_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <memory>
#include <future>
#include <utility>
#include <string>
#include <string_view>
#include <chrono>

#include <asio2/external/asio.hpp>
#include <asio2/bho/predef.h>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>
#include <asio2/base/detail/util.hpp>

#include <asio2/udp/detail/ikcp.h>

namespace asio2::detail::kcp
{
//	struct psdhdr
//	{
//		unsigned long saddr;
//		unsigned long daddr;
//		char mbz;
//		char ptcl;
//		unsigned short tcpl;
//	};
//
//	struct tcphdr
//	{
//		std::uint16_t th_sport;        /* source port */
//		std::uint16_t th_dport;        /* destination port */
//		std::uint32_t th_seq;          /* sequence number */
//		std::uint32_t th_ack;          /* acknowledgement number */
//#if IWORDS_BIG_ENDIAN
//		std::uint16_t th_off : 4;      /* data offset */
//		std::uint16_t th_x2 : 6;       /* (unused) */
//#else
//		std::uint16_t th_x2 : 6;       /* (unused) */
//		std::uint16_t th_off : 4;      /* data offset */
//#endif
//		std::uint16_t thf_urg : 1;     /* flags : Urgent Pointer Field Significant */
//		std::uint16_t thf_ack : 1;     /* flags : Acknowledgement field significant */
//		std::uint16_t thf_psh : 1;     /* flags : Push Function */
//		std::uint16_t thf_rst : 1;     /* flags : Reset the connection */
//		std::uint16_t thf_syn : 1;     /* flags : Synchronize sequence numbers */
//		std::uint16_t thf_fin : 1;     /* flags : No more data from sender */
//		std::uint16_t th_win;          /* window */
//		std::uint16_t th_sum;          /* checksum */
//		std::uint16_t th_urp;          /* urgent pointer */
//		std::uint32_t th_option : 24;  /* option */
//		std::uint32_t th_padding : 8;  /* padding */
//	};

	struct kcphdr
	{
		std::uint32_t th_seq{};
		std::uint32_t th_ack{};
		union
		{
			std::uint8_t      byte{};
		#if BHO_ENDIAN_BIG_BYTE
			struct
			{
				std::uint8_t  urg     : 1;
				std::uint8_t  ack     : 1;
				std::uint8_t  psh     : 1;
				std::uint8_t  rst     : 1;
				std::uint8_t  syn     : 1;
				std::uint8_t  fin     : 1;
				std::uint8_t  padding : 2;
			} bits;
		#else
			struct
			{
				std::uint8_t  padding : 2;
				std::uint8_t  fin     : 1;
				std::uint8_t  syn     : 1;
				std::uint8_t  rst     : 1;
				std::uint8_t  psh     : 1;
				std::uint8_t  ack     : 1;
				std::uint8_t  urg     : 1;
			} bits;
		#endif
		}             th_flag   {};
		std::uint8_t  th_padding{};
		std::uint16_t th_sum    {};

		static constexpr std::size_t required_size() noexcept
		{
			return (0
				+ sizeof(std::uint32_t) // std::uint32_t th_seq;
				+ sizeof(std::uint32_t) // std::uint32_t th_ack;
				+ sizeof(std::uint8_t )	// std::uint8_t  th_flag;
				+ sizeof(std::uint8_t )	// std::uint8_t  th_padding;
				+ sizeof(std::uint16_t) // std::uint16_t th_sum;
				);
		}
	};

	template<typename = void>
	std::string to_string(kcphdr& hdr)
	{
		std::string s/*{ kcphdr::required_size(), '\0' }*/;

		s.resize(kcphdr::required_size());

		std::string::pointer p = s.data();

		write(p, hdr.th_seq      );
		write(p, hdr.th_ack      );
		write(p, hdr.th_flag.byte);
		write(p, hdr.th_padding  );
		write(p, hdr.th_sum      );

		return s;
	}

	template<typename = void>
	kcphdr to_kcphdr(std::string_view s) noexcept
	{
		kcphdr hdr{};

		std::string_view::pointer p = const_cast<std::string_view::pointer>(s.data());

		if (s.size() >= kcphdr::required_size())
		{
			hdr.th_seq       = read<std::uint32_t>(p);
			hdr.th_ack       = read<std::uint32_t>(p);
			hdr.th_flag.byte = read<std::uint8_t >(p);
			hdr.th_padding   = read<std::uint8_t >(p);
			hdr.th_sum       = read<std::uint16_t>(p);
		}

		return hdr;
	}

	template<typename = void>
	unsigned short checksum(unsigned short * addr, int size) noexcept
	{
		long sum = 0;
		while (size > 1)
		{
			sum += *(unsigned short*)addr++;
			size -= 2;
		}
		if (size > 0)
		{
			std::uint8_t left_over[2] = { 0 };
			left_over[0] = static_cast<std::uint8_t>(*addr);
			sum += *(unsigned short*)left_over;
		}
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);
		return static_cast<unsigned short>(~sum);
	}

	template<typename = void>
	inline bool is_kcphdr_syn(std::string_view s) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg && !hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst &&  hdr.th_flag.bits.syn && !hdr.th_flag.bits.fin))
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_synack(std::string_view s, std::uint32_t seq) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg && hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst && hdr.th_flag.bits.syn && !hdr.th_flag.bits.fin))
			return false;

		if (hdr.th_ack != seq + 1)
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_ack(std::string_view s, std::uint32_t seq) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg &&  hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst && !hdr.th_flag.bits.syn && !hdr.th_flag.bits.fin))
			return false;

		if (hdr.th_ack != seq + 1)
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_fin(std::string_view s) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg && !hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst && !hdr.th_flag.bits.syn &&  hdr.th_flag.bits.fin))
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline kcphdr make_kcphdr_syn(std::uint32_t seq)
	{
		kcphdr hdr{};
		hdr.th_seq = seq;
		hdr.th_flag.bits.syn = 1;

		std::string s = to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_synack(std::uint32_t seq, std::uint32_t ack)
	{
		kcphdr hdr{};
		hdr.th_seq = seq;
		hdr.th_ack = ack + 1;
		hdr.th_flag.bits.ack = 1;
		hdr.th_flag.bits.syn = 1;

		std::string s = to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_ack(std::uint32_t ack)
	{
		kcphdr hdr{};
		hdr.th_ack = ack + 1;
		hdr.th_flag.bits.ack = 1;

		std::string s = to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_fin(std::uint32_t seq)
	{
		kcphdr hdr{};
		hdr.th_seq = seq;
		hdr.th_flag.bits.fin = 1;

		std::string s = to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	struct kcp_deleter
	{
		inline void operator()(ikcpcb* p) const noexcept { kcp::ikcp_release(p); };
	};
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_KCP_UTIL_HPP__
