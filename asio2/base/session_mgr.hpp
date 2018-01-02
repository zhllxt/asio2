/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_SESSION_MGR_HPP__
#define __ASIO2_SESSION_MGR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <chrono>

#include <boost/asio.hpp>

#include <asio2/util/rwlock.hpp>

namespace asio2
{

	class session_impl;

	class session_mgr
	{
	public:
		/**
		 * @construct
		 */
		explicit session_mgr()
		{
		}

		/**
		 * @destruct
		 */
		virtual ~session_mgr()
		{
		}

		/**
		 * @function : start and emplace session_impl
		 */
		virtual bool start(const std::shared_ptr<session_impl> & session_ptr) = 0;

		/**
		 * @function : stop and erase session_impl
		 */
		virtual void stop(const std::shared_ptr<session_impl> & session_ptr) = 0;

		/**
		 * @function : stop all sessions
		 */
		virtual void stop_all() = 0;

		/**
		 * @function : call user custom function for every session_impl in the session_impl map
		 * the handler is like this :
		 * void on_callback(std::shared_ptr<session_impl> & session_ptr)
		 */
		virtual void for_each_session(const std::function<void(std::shared_ptr<session_impl> & session_ptr)> & handler) = 0;

		/**
		 * @function : find the session_impl by map key
		 * @return   : session_impl shared_ptr reference
		 */
		virtual std::shared_ptr<session_impl> & find_session(void * key) = 0;

		/**
		 * @function : get session_impl count
		 */
		virtual std::size_t get_session_count() = 0;
	};

}

#endif // !__ASIO2_SESSION_MGR_HPP__
