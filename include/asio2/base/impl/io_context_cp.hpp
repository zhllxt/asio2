/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_IO_CONTEXT_COMPONENT_HPP__
#define __ASIO2_IO_CONTEXT_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/iopool.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class io_context_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		explicit io_context_cp(std::shared_ptr<io_t> rwio) : io_(std::move(rwio))
		{
		}

		/**
		 * @brief destructor
		 */
		~io_context_cp()
		{
		}

		/**
		 * @brief get the io object reference
		 */
		inline io_t& io() noexcept
		{
			return *(this->io_);
		}

		/**
		 * @brief get the io object reference
		 */
		inline io_t const& io() const noexcept
		{
			return *(this->io_);
		}

		/**
		 * @brief get the io object shared_ptr
		 */
		inline std::shared_ptr<io_t> io_ptr() noexcept
		{
			return this->io_;
		}

	protected:
		/// The io_context wrapper used to handle the recv/send event.
		/// the io_context should be destroyed after socket, beacuse the socket used the
		/// io_context, so if the io_context is destroyed before socket, then maybe cause crash.
		std::shared_ptr<io_t>                io_;
	};
}

#endif // !__ASIO2_IO_CONTEXT_COMPONENT_HPP__
