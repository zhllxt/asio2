/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_ERROR_HPP__
#define __ASIO2_ERROR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <iostream>
#include <memory>
#include <cerrno>

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>

#include <asio2/util/logger.hpp>

namespace asio2
{

	// use anonymous namespace to resolve global function redefinition problem
	namespace
	{

		/**
		 * thread local variable of error number.
		 */
		thread_local static int _err_num = 0;

		thread_local static std::string _err_msg;


		/**
		 * @function : get last error number
		 */
		inline int get_last_error()
		{
			return _err_num;
		}

		/**
		 * @function : set last error number and desc
		 */
		inline void set_last_error(int err_num, const std::string & err_msg)
		{
			_err_num = err_num;
			_err_msg = err_msg;
		}

		/**
		 * @function : set last error number and desc
		 */
		inline void set_last_error(int err_num, const char * err_msg = "")
		{
			_err_num = err_num;
			_err_msg = err_msg;
		}

		/**
		 * @function : get last error desc
		 */
		inline std::string get_last_error_desc()
		{
			if (_err_msg.empty())
				return boost::system::error_code(_err_num, boost::system::system_category()).message();
			return _err_msg;
		}

		/**
		 * @function : get the specify error desc
		 */
		inline std::string get_error_desc(int err_num)
		{
			return boost::system::error_code(err_num, boost::system::system_category()).message();
		}

	}


}

#endif // !__ASIO2_ERROR_HPP__
