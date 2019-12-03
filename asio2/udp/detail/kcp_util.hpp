/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <memory>
#include <future>
#include <utility>
#include <string_view>
#include <chrono>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/udp/detail/ikcp.h>

namespace asio2::detail::kcp
{
	// __attribute__((aligned(1)))
#if defined(__GNUC__) || defined(__GNUG__)
	#define __KCPHDR_ONEBYTE_ALIGN__ __attribute__((packed))
#elif defined(_MSC_VER)
	#define __KCPHDR_ONEBYTE_ALIGN__
	#pragma pack(push,1)
#endif
//	struct psdhdr
//	{
//		unsigned long saddr;
//		unsigned long daddr;
//		char mbz;
//		char ptcl;
//		unsigned short tcpl;
//	}__KCPHDR_ONEBYTE_ALIGN__;
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
//	}__KCPHDR_ONEBYTE_ALIGN__;

	struct kcphdr
	{
		std::uint32_t th_seq;
		std::uint32_t th_ack;
		std::uint16_t thf_urg : 1;
		std::uint16_t thf_ack : 1;
		std::uint16_t thf_psh : 1;
		std::uint16_t thf_rst : 1;
		std::uint16_t thf_syn : 1;
		std::uint16_t thf_fin : 1;
		std::uint16_t th_padding : 10;
		std::uint16_t th_sum;
	}__KCPHDR_ONEBYTE_ALIGN__;
#if defined(__GNUC__) || defined(__GNUG__)
	#undef __KCPHDR_ONEBYTE_ALIGN__
#elif defined(_MSC_VER)
	#pragma pack(pop)
	#undef __KCPHDR_ONEBYTE_ALIGN__
#endif

	template<typename = void>
	unsigned short checksum(unsigned short * addr, int count)
	{
		long sum = 0;
		while (count > 1)
		{
			sum += *(unsigned short*)addr++;
			count -= 2;
		}
		if (count > 0)
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
	inline bool is_kcphdr_syn(std::string_view s)
	{
		if (s.size() != sizeof(kcphdr))
			return false;

		kcphdr * hdr = (kcphdr*)(s.data());
		if (!(!hdr->thf_urg && !hdr->thf_ack && !hdr->thf_psh && !hdr->thf_rst && hdr->thf_syn && !hdr->thf_fin))
			return false;

		return (hdr->th_sum == checksum(reinterpret_cast<unsigned short *>(hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_synack(std::string_view s, std::uint32_t seq)
	{
		if (s.size() != sizeof(kcphdr))
			return false;

		kcphdr * hdr = (kcphdr*)(s.data());
		if (!(!hdr->thf_urg && hdr->thf_ack && !hdr->thf_psh && !hdr->thf_rst && hdr->thf_syn && !hdr->thf_fin))
			return false;

		if (hdr->th_ack != seq + 1)
			return false;

		return (hdr->th_sum == checksum(reinterpret_cast<unsigned short *>(hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_ack(std::string_view s, std::uint32_t seq)
	{
		if (s.size() != sizeof(kcphdr))
			return false;

		kcphdr * hdr = (kcphdr*)(s.data());
		if (!(!hdr->thf_urg && hdr->thf_ack && !hdr->thf_psh && !hdr->thf_rst && !hdr->thf_syn && !hdr->thf_fin))
			return false;

		if (hdr->th_ack != seq + 1)
			return false;

		return (hdr->th_sum == checksum(reinterpret_cast<unsigned short *>(hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline bool is_kcphdr_fin(std::string_view s)
	{
		if (s.size() != sizeof(kcphdr))
			return false;

		kcphdr * hdr = (kcphdr*)(s.data());
		if (!(!hdr->thf_urg && !hdr->thf_ack && !hdr->thf_psh && !hdr->thf_rst && !hdr->thf_syn && hdr->thf_fin))
			return false;

		return (hdr->th_sum == checksum(reinterpret_cast<unsigned short *>(hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum))));
	}

	template<typename = void>
	inline kcphdr make_kcphdr_syn(std::uint32_t seq)
	{
		kcphdr hdr = { 0 };
		hdr.th_seq = seq;
		hdr.thf_syn = 1;
		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(&hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_synack(std::uint32_t seq, std::uint32_t ack)
	{
		kcphdr hdr = { 0 };
		hdr.th_seq = seq;
		hdr.th_ack = ack + 1;
		hdr.thf_ack = 1;
		hdr.thf_syn = 1;
		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(&hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_ack(std::uint32_t ack)
	{
		kcphdr hdr = { 0 };
		hdr.th_ack = ack + 1;
		hdr.thf_ack = 1;
		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(&hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	template<typename = void>
	inline kcphdr make_kcphdr_fin(std::uint32_t seq)
	{
		kcphdr hdr = { 0 };
		hdr.th_seq = seq;
		hdr.thf_fin = 1;
		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(&hdr),
			static_cast<int>(sizeof(kcphdr) - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	struct kcp_deleter
	{
		inline void operator()(ikcpcb* p) const { kcp::ikcp_release(p); };
	};
}

#endif // !__ASIO2_KCP_UTIL_HPP__
