/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

#include <asio2/external/predef.h>

#include <asio2/base/error.hpp>
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
//		::std::uint16_t th_sport;        /* source port */
//		::std::uint16_t th_dport;        /* destination port */
//		::std::uint32_t th_seq;          /* sequence number */
//		::std::uint32_t th_ack;          /* acknowledgement number */
//#if IWORDS_BIG_ENDIAN
//		::std::uint16_t th_off : 4;      /* data offset */
//		::std::uint16_t th_x2 : 6;       /* (unused) */
//#else
//		::std::uint16_t th_x2 : 6;       /* (unused) */
//		::std::uint16_t th_off : 4;      /* data offset */
//#endif
//		::std::uint16_t thf_urg : 1;     /* flags : Urgent Pointer Field Significant */
//		::std::uint16_t thf_ack : 1;     /* flags : Acknowledgement field significant */
//		::std::uint16_t thf_psh : 1;     /* flags : Push Function */
//		::std::uint16_t thf_rst : 1;     /* flags : Reset the connection */
//		::std::uint16_t thf_syn : 1;     /* flags : Synchronize sequence numbers */
//		::std::uint16_t thf_fin : 1;     /* flags : No more data from sender */
//		::std::uint16_t th_win;          /* window */
//		::std::uint16_t th_sum;          /* checksum */
//		::std::uint16_t th_urp;          /* urgent pointer */
//		::std::uint32_t th_option : 24;  /* option */
//		::std::uint32_t th_padding : 8;  /* padding */
//	};

	struct kcphdr
	{
		::std::uint32_t th_seq{};
		::std::uint32_t th_ack{};
		union
		{
			::std::uint8_t      byte{};
		#if ASIO2_ENDIAN_BIG_BYTE
			struct
			{
				::std::uint8_t  urg     : 1;
				::std::uint8_t  ack     : 1;
				::std::uint8_t  psh     : 1;
				::std::uint8_t  rst     : 1;
				::std::uint8_t  syn     : 1;
				::std::uint8_t  fin     : 1;
				::std::uint8_t  padding : 2;
			} bits;
		#else
			struct
			{
				::std::uint8_t  padding : 2;
				::std::uint8_t  fin     : 1;
				::std::uint8_t  syn     : 1;
				::std::uint8_t  rst     : 1;
				::std::uint8_t  psh     : 1;
				::std::uint8_t  ack     : 1;
				::std::uint8_t  urg     : 1;
			} bits;
		#endif
		}               th_flag   {};
		::std::uint8_t  th_padding{};
		::std::uint16_t th_sum    {};

		static constexpr ::std::size_t required_size() noexcept
		{
			return (0
				+ sizeof(::std::uint32_t) // ::std::uint32_t th_seq;
				+ sizeof(::std::uint32_t) // ::std::uint32_t th_ack;
				+ sizeof(::std::uint8_t ) // ::std::uint8_t  th_flag;
				+ sizeof(::std::uint8_t ) // ::std::uint8_t  th_padding;
				+ sizeof(::std::uint16_t) // ::std::uint16_t th_sum;
				);
		}
	};

	struct kcp_deleter
	{
		inline void operator()(ikcpcb* p) const noexcept { kcp::ikcp_release(p); };
	};

	namespace
	{
	::std::string to_string(kcphdr& hdr)
	{
		::std::string s/*{ kcphdr::required_size(), '\0' }*/;

		s.resize(kcphdr::required_size());

		::std::string::pointer p = s.data();

		detail::write(p, hdr.th_seq      );
		detail::write(p, hdr.th_ack      );
		detail::write(p, hdr.th_flag.byte);
		detail::write(p, hdr.th_padding  );
		detail::write(p, hdr.th_sum      );

		return s;
	}

	kcphdr to_kcphdr(::std::string_view s) noexcept
	{
		kcphdr hdr{};

		::std::string_view::pointer p = const_cast<::std::string_view::pointer>(s.data());

		if (s.size() >= kcphdr::required_size())
		{
			hdr.th_seq       = detail::read<::std::uint32_t>(p);
			hdr.th_ack       = detail::read<::std::uint32_t>(p);
			hdr.th_flag.byte = detail::read<::std::uint8_t >(p);
			hdr.th_padding   = detail::read<::std::uint8_t >(p);
			hdr.th_sum       = detail::read<::std::uint16_t>(p);
		}

		return hdr;
	}

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
			::std::uint8_t left_over[2] = { 0 };
			left_over[0] = static_cast<::std::uint8_t>(*addr);
			sum += *(unsigned short*)left_over;
		}
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);
		return static_cast<unsigned short>(~sum);
	}

	inline bool is_kcphdr_syn(::std::string_view s) noexcept
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

	inline bool is_kcphdr_synack(::std::string_view s, ::std::uint32_t seq, bool ignore_seq = false) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg && hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst && hdr.th_flag.bits.syn && !hdr.th_flag.bits.fin))
			return false;

		if (!ignore_seq && hdr.th_ack != seq + 1)
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	inline bool is_kcphdr_ack(::std::string_view s, ::std::uint32_t seq, bool ignore_seq = false) noexcept
	{
		if (s.size() != kcphdr::required_size())
			return false;

		kcphdr hdr = to_kcphdr(s);
		if (!(
			!hdr.th_flag.bits.urg &&  hdr.th_flag.bits.ack && !hdr.th_flag.bits.psh &&
			!hdr.th_flag.bits.rst && !hdr.th_flag.bits.syn && !hdr.th_flag.bits.fin))
			return false;

		if (!ignore_seq && hdr.th_ack != seq + 1)
			return false;

		return (hdr.th_sum == checksum((unsigned short *)(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum))));
	}

	inline bool is_kcphdr_fin(::std::string_view s) noexcept
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

	inline kcphdr make_kcphdr_syn(::std::uint32_t conv, ::std::uint32_t seq)
	{
		kcphdr hdr{};
		hdr.th_seq = seq;
		hdr.th_ack = conv;
		hdr.th_flag.bits.syn = 1;

		::std::string s = kcp::to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	inline kcphdr make_kcphdr_synack(::std::uint32_t conv, ::std::uint32_t ack)
	{
		kcphdr hdr{};
		hdr.th_seq = conv;
		hdr.th_ack = ack + 1;
		hdr.th_flag.bits.ack = 1;
		hdr.th_flag.bits.syn = 1;

		::std::string s = kcp::to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	inline kcphdr make_kcphdr_ack(::std::uint32_t ack)
	{
		kcphdr hdr{};
		hdr.th_ack = ack + 1;
		hdr.th_flag.bits.ack = 1;

		::std::string s = kcp::to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	inline kcphdr make_kcphdr_fin(::std::uint32_t seq)
	{
		kcphdr hdr{};
		hdr.th_seq = seq;
		hdr.th_flag.bits.fin = 1;

		::std::string s = kcp::to_string(hdr);

		hdr.th_sum = checksum(reinterpret_cast<unsigned short *>(s.data()),
			static_cast<int>(kcphdr::required_size() - sizeof(kcphdr::th_sum)));

		return hdr;
	}

	[[maybe_unused]] void ikcp_reset(ikcpcb* kcp)
	{
		//#### ikcp_release without free
		assert(kcp);
		if (kcp) {
			IKCPSEG* seg;
			while (!iqueue_is_empty(&kcp->snd_buf)) {
				seg = iqueue_entry(kcp->snd_buf.next, IKCPSEG, node);
				iqueue_del(&seg->node);
				ikcp_segment_delete(kcp, seg);
			}
			while (!iqueue_is_empty(&kcp->rcv_buf)) {
				seg = iqueue_entry(kcp->rcv_buf.next, IKCPSEG, node);
				iqueue_del(&seg->node);
				ikcp_segment_delete(kcp, seg);
			}
			while (!iqueue_is_empty(&kcp->snd_queue)) {
				seg = iqueue_entry(kcp->snd_queue.next, IKCPSEG, node);
				iqueue_del(&seg->node);
				ikcp_segment_delete(kcp, seg);
			}
			while (!iqueue_is_empty(&kcp->rcv_queue)) {
				seg = iqueue_entry(kcp->rcv_queue.next, IKCPSEG, node);
				iqueue_del(&seg->node);
				ikcp_segment_delete(kcp, seg);
			}
			//if (kcp->buffer) {
			//	ikcp_free(kcp->buffer);
			//}
			if (kcp->acklist) {
				ikcp_free(kcp->acklist);
			}

			kcp->nrcv_buf = 0;
			kcp->nsnd_buf = 0;
			kcp->nrcv_que = 0;
			kcp->nsnd_que = 0;
			kcp->ackcount = 0;
			//kcp->buffer = NULL;
			kcp->acklist = NULL;
			//ikcp_free(kcp);
		}

		//#### ikcp_create without malloc
		if (kcp) {
			//ikcpcb* kcp = (ikcpcb*)ikcp_malloc(sizeof(struct IKCPCB));
			//if (kcp == NULL) return NULL;
			//kcp->conv = conv;
			//kcp->user = user;
			kcp->snd_una = 0;
			kcp->snd_nxt = 0;
			kcp->rcv_nxt = 0;
			kcp->ts_recent = 0;
			kcp->ts_lastack = 0;
			kcp->ts_probe = 0;
			kcp->probe_wait = 0;
			//kcp->snd_wnd = IKCP_WND_SND; // ikcp_wndsize
			//kcp->rcv_wnd = IKCP_WND_RCV; // ikcp_wndsize
			kcp->rmt_wnd = IKCP_WND_RCV;
			kcp->cwnd = 0;
			kcp->incr = 0;
			kcp->probe = 0;
			//kcp->mtu = IKCP_MTU_DEF;             // ikcp_setmtu
			//kcp->mss = kcp->mtu - IKCP_OVERHEAD; // ikcp_setmtu
			kcp->stream = 0;

			//kcp->buffer = (char*)ikcp_malloc((kcp->mtu + IKCP_OVERHEAD) * 3);
			//if (kcp->buffer == NULL) {
			//	ikcp_free(kcp);
			//	return NULL;
			//}

			iqueue_init(&kcp->snd_queue);
			iqueue_init(&kcp->rcv_queue);
			iqueue_init(&kcp->snd_buf);
			iqueue_init(&kcp->rcv_buf);
			kcp->nrcv_buf = 0;
			kcp->nsnd_buf = 0;
			kcp->nrcv_que = 0;
			kcp->nsnd_que = 0;
			kcp->state = 0;
			kcp->acklist = NULL;
			kcp->ackblock = 0;
			kcp->ackcount = 0;
			kcp->rx_srtt = 0;
			kcp->rx_rttval = 0;
			kcp->rx_rto = IKCP_RTO_DEF;
			//kcp->rx_minrto = IKCP_RTO_MIN; // ikcp_nodelay
			kcp->current = 0;
			//kcp->interval = IKCP_INTERVAL; // ikcp_nodelay
			kcp->ts_flush = IKCP_INTERVAL;
			//kcp->nodelay = 0;              // ikcp_nodelay
			kcp->updated = 0;
			//kcp->logmask = 0;
			kcp->ssthresh = IKCP_THRESH_INIT;
			//kcp->fastresend = 0;           // ikcp_nodelay
			//kcp->fastlimit = IKCP_FASTACK_LIMIT;
			//kcp->nocwnd = 0;               // ikcp_nodelay
			kcp->xmit = 0;
			//kcp->dead_link = IKCP_DEADLINK;
			//kcp->output = NULL;
			//kcp->writelog = NULL;
		}
	}
	}
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_KCP_UTIL_HPP__
