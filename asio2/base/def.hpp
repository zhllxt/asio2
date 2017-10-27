/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_DEF_HPP__
#define __ASIO2_DEF_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)


namespace asio2
{

	//---------------------------------------------------------------------------------------------
	// exception log
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_EXCEPTION_CODE (boost::asio::error::invalid_argument)

	#if defined(PRINT_FATAL_EXCEPTION)
		//#define PRINT_EXCEPTION std::cerr << "fatal exception (code - " << get_last_error() << ") : \"" << get_last_error_desc() << "\" " << __FILE__ << " " << __LINE__ << std::endl;
		#define PRINT_EXCEPTION asio2::logger::get().write("fatal exception (code - %d) : \"%s\" %s %d\n", get_last_error(), get_last_error_desc().c_str(), __FILE__, __LINE__);
	#else
		#define PRINT_EXCEPTION 
	#endif



	//---------------------------------------------------------------------------------------------
	// pack model used macro
	//---------------------------------------------------------------------------------------------

	// mean that current data length is not enough for a completed packet,it need more data
	#define NEED_MORE_DATA (std::size_t(-1))

	// mean that current data is invalid,may be the client is a invalid or hacker client,if get this 
	// return value,the session will be disconnect forced.
	#define INVALID_DATA   (std::size_t(-2))



	//---------------------------------------------------------------------------------------------
	// auto model used macro
	//---------------------------------------------------------------------------------------------

	#define DEFAULT_HEADER_FLAG        ((uint8_t)0b10101010)
	#define DEFAULT_MAX_HEADER_FLAG    ((uint8_t)0xff)
	#define DEFAULT_MAX_PACKET_SIZE    ((uint32_t)0x00ffffff)
	#define PACKET_SIZE_MASK           ((uint32_t)0xffffff00)
	#define HEADER_FLAG_MASK           ((uint32_t)0x000000ff)
	#define PACKET_SIZE_OFFSET_BITS    (8)



	//---------------------------------------------------------------------------------------------
	// udp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_SILENCE_TIMEOUT    (60)



	//---------------------------------------------------------------------------------------------
	// icmp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_REPLY_TIMEOUT      (5000)

}

#endif // !__ASIO2_DEF_HPP__
