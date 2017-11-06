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

	#if defined(PRINT_FATAL_EXCEPTION)
	//	#define PRINT_EXCEPTION std::cerr << "fatal exception (code - " << get_last_error() << ") : \"" << get_last_error_desc() << "\" " << __FILE__ << " " << __LINE__ << std::endl;
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

	#define DEFAULT_HEADER_FLAG        ((uint8_t)0b011101) // 29
	#define MAX_HEADER_FLAG            ((uint8_t)0b111111) // 63
	#define MAX_PACKET_SIZE            ((uint32_t)0b00000011111111111111111111111111) // 0x03FF FFFF
	#define HEADER_FLAG_MASK           ((uint32_t)0b00000000000000000000000000111111)
	#define HEADER_FLAG_BITS           (6)



	//---------------------------------------------------------------------------------------------
	// udp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_SILENCE_TIMEOUT    (60)



	//---------------------------------------------------------------------------------------------
	// icmp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_REPLY_TIMEOUT      (5000)



	//---------------------------------------------------------------------------------------------
	// http used macro
	//---------------------------------------------------------------------------------------------
	#define HTTP_ERROR_CODE_MASK       (0x400000) // http_parser error code,we manual set the 22 bit to 1

}

#endif // !__ASIO2_DEF_HPP__
