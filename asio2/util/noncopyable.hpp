/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * refrenced from boost/noncopyable.hpp
 *
 */

#ifndef __ASIO2_NONCOPYABLE_HPP__
#define __ASIO2_NONCOPYABLE_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

namespace asio2
{

	class noncopyable
	{
	protected:
#if __cplusplus >= 201103L
		constexpr noncopyable() = default;
		~noncopyable() = default;
#else
		noncopyable() {}
		~noncopyable() {}
#endif

#if __cplusplus >= 201103L
		noncopyable(const noncopyable&) = delete;
		noncopyable& operator=(const noncopyable&) = delete;
#else
	private:  // emphasize the following members are private
		noncopyable(const noncopyable&);
		noncopyable& operator=(const noncopyable&);
#endif
	};

}

#endif  // !__ASIO2_NONCOPYABLE_HPP__
