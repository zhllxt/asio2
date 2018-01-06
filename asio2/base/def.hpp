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
	// global macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_SEND_BUFFER_SIZE             (1024)
	#define DEFAULT_RECV_BUFFER_SIZE             (1024)

	enum class state
	{
		stopped,
		stopping,
		starting,
		started,
		running,
	};


	//---------------------------------------------------------------------------------------------
	// exception log
	//---------------------------------------------------------------------------------------------

	#if defined(ASIO2_WRITE_LOG)
		#define PRINT_EXCEPTION asio2::logger::get().log_trace("exception (errno - %d) : \"%s\" %s %d\n", get_last_error(), get_last_error_desc().data(), __FILE__, __LINE__);
	#else
		#define PRINT_EXCEPTION 
	#endif



	//---------------------------------------------------------------------------------------------
	// tcp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_TCP_SILENCE_TIMEOUT             (60 * 60)
	#define DEFAULT_SSL_SHUTDOWN_TIMEOUT            (10)



	//---------------------------------------------------------------------------------------------
	// pack model used macro
	//---------------------------------------------------------------------------------------------

	// mean that current data length is not enough for a completed packet,it need more data
	static const std::size_t need_more_data = (std::size_t(-1));

	// mean that current data is invalid,may be the client is a invalid or hacker client,if get this 
	// return value,the session will be disconnect forced.
	static const std::size_t invalid_data   = (std::size_t(-2));



	//---------------------------------------------------------------------------------------------
	// auto model used macro
	//---------------------------------------------------------------------------------------------

	#define DEFAULT_HEADER_FLAG                     ((uint8_t)0b011101) // 29
	#define MAX_HEADER_FLAG                         ((uint8_t)0b111111) // 63
	#define MAX_PACKET_SIZE                         ((uint32_t)0b00000011111111111111111111111111) // 0x03FF FFFF
	#define HEADER_FLAG_MASK                        ((uint32_t)0b00000000000000000000000000111111)
	#define HEADER_FLAG_BITS                        (6)



	//---------------------------------------------------------------------------------------------
	// udp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_UDP_SILENCE_TIMEOUT             (60)



	//---------------------------------------------------------------------------------------------
	// icmp used macro
	//---------------------------------------------------------------------------------------------
	#define DEFAULT_ICMP_REPLY_TIMEOUT              (5000)



	//---------------------------------------------------------------------------------------------
	// http used macro
	//---------------------------------------------------------------------------------------------
	#define HTTP_ERROR_CODE_MASK                    (0x400000) // http_parser error code,we manual set the 22 bit to 1
	#define DEFAULT_HTTP_KEEPALIVE_TIMEOUT          (10)
	#define DEFAULT_HTTP_SILENCE_TIMEOUT            (1)
	#define DEFAULT_HTTP_MAX_REQUEST_COUNT          (100)

}

#endif // !__ASIO2_DEF_HPP__
