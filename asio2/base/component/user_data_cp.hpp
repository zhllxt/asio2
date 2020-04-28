/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_USER_DATA_COMPONENT_HPP__
#define __ASIO2_USER_DATA_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <any>

namespace asio2::detail
{
	template<class derived_t>
	class user_data_cp
	{
	public:
		/**
		 * @constructor
		 */
		user_data_cp() {}

		/**
		 * @destructor
		 */
		~user_data_cp() = default;

	public:
		/**
		 * @function : set user data, internal use std::any to storage, you can set any type of data
		 */
		template<class DataT>
		inline derived_t & user_data(DataT && data)
		{
			this->user_data_ = std::forward<DataT>(data);
			return (static_cast<derived_t &>(*this));
		}

		/**
		 * @function : get user data
		 * example : MyStruct my = user_data<MyStruct>(); MyStruct* my = user_data<MyStruct*>();
		 */
		template<class DataT>
		inline DataT user_data()
		{
			try
			{
				if constexpr (std::is_pointer_v<DataT>)
					return std::any_cast<std::remove_pointer_t<DataT>>(&(this->user_data_));
				else
					return std::any_cast<DataT>(this->user_data_);
			}
			catch (const std::bad_any_cast&) {}
			return DataT{};
		}

	protected:
		/// user data
		std::any user_data_;
	};
}

#endif // !__ASIO2_USER_DATA_COMPONENT_HPP__
