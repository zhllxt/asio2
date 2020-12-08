/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * The cost of dynamic (virtual calls) vs. static (CRTP) dispatch in C++
 * https://eli.thegreenplace.net/2013/12/05/the-cost-of-dynamic-virtual-calls-vs-static-crtp-dispatch-in-c/
 */

#ifndef __ASIO2_OBJECT_HPP__
#define __ASIO2_OBJECT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>

#include <asio2/base/error.hpp>

namespace asio2::detail
{
	/**
	 * the lowest based class used fo CRTP
	 * see : CRTP and multilevel inheritance 
	 * https://stackoverflow.com/questions/18174441/crtp-and-multilevel-inheritance
	 */
	template<class derived_t, bool enable_shared_from_this = true>
	class object_t : public std::enable_shared_from_this<derived_t>
	{
	protected:
		/**
		 * @constructor
		 */
		object_t() = default;

		/**
		 * @destructor
		 */
		~object_t() = default;

	protected:
		/**
		 * @function : obtain derived class object through CRTP mechanism
		 */
		inline const derived_t & derived() const
		{
			return static_cast<const derived_t &>(*this);
		}

		/**
		 * @function : obtain derived class object through CRTP mechanism
		 */
		inline derived_t & derived()
		{
			return static_cast<derived_t &>(*this);
		}

		inline std::shared_ptr<derived_t> selfptr()
		{
			try
			{
				return this->derived().shared_from_this();
			}
			catch (const std::bad_weak_ptr&)
			{
				ASIO2_ASSERT(false);
			}
			catch (const std::exception&)
			{
				ASIO2_ASSERT(false);
			}

			throw std::bad_weak_ptr{};
		}
	};

	template<class derived_t>
	class object_t<derived_t, false>
	{
	protected:
		/**
		 * @constructor
		 */
		object_t() = default;

		/**
		 * @destructor
		 */
		~object_t() = default;

	protected:
		/**
		 * @function : obtain derived class object through CRTP mechanism
		 */
		inline const derived_t & derived() const
		{
			return static_cast<const derived_t &>(*this);
		}

		/**
		 * @function : obtain derived class object through CRTP mechanism
		 */
		inline derived_t & derived()
		{
			return static_cast<derived_t &>(*this);
		}

		inline std::shared_ptr<derived_t> selfptr()
		{
			return std::shared_ptr<derived_t>{};
		}
	};
}

#endif // !__ASIO2_OBJECT_HPP__
