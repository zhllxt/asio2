/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_HTTP_REQUEST_HANDLER_HPP__
#define __ASIO2_HTTP_REQUEST_HANDLER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <unordered_map>

#include <asio2/http/http_parser.h>
#include <asio2/http/mime_types.hpp>


namespace asio2
{

	class http_request_handler
	{
	public:

		/**
		 * @construct
		 */
		explicit http_request_handler()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~http_request_handler()
		{
		}


	protected:

	};

}

#endif // !__ASIO2_HTTP_REQUEST_HANDLER_HPP__
