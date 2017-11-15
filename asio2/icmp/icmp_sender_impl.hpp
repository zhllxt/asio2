/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * Author   : zhllxt
 * QQ       : 37792738
 * Email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_ICMP_SENDER_IMPL_HPP__
#define __ASIO2_ICMP_SENDER_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <thread>
#include <atomic>

#include <boost/asio.hpp>

#include <asio2/util/pool.hpp>
#include <asio2/util/helper.hpp>

#include <asio2/base/sender_impl.hpp>
#include <asio2/base/io_service_pool.hpp>
#include <asio2/base/listener_mgr.hpp>

#include <asio2/icmp/icmp_header.hpp>
#include <asio2/icmp/ipv4_header.hpp>

namespace asio2
{

	/**
	 * the icmp sender impl 
	 * you must construct this object by "new" mode, can't construct this object on the stack.because it used shared_from_this.
	 */
	class icmp_sender_impl : public sender_impl
	{
	public:


	};
}

#endif // !__ASIO2_ICMP_SENDER_IMPL_HPP__
