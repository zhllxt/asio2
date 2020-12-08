/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__
#define __ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <future>
#include <queue>
#include <tuple>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t = void>
	class data_persistence_cp
	{
	public:
		/**
		 * @constructor
		 */
		data_persistence_cp() {}

		/**
		 * @destructor
		 */
		~data_persistence_cp() = default;

	protected:
		template<class T>
		inline auto _data_persistence(T&& data)
		{
			using data_type = std::remove_cv_t<std::remove_reference_t<T>>;
			if constexpr (
				std::is_move_assignable_v<data_type> ||
				std::is_trivially_move_assignable_v<data_type> ||
				std::is_nothrow_move_assignable_v<data_type>)
			{
				if constexpr (is_string_view_v<data_type>)
				{
					using value_type = typename data_type::value_type;
					return std::basic_string<value_type>(data.data(), data.size());
				}
				else
				{
					return std::forward<T>(data);
				}
			}
			else
			{
				auto buffer = asio::buffer(data);
				return std::string(reinterpret_cast<const std::string::value_type*>(
					const_cast<const void*>(buffer.data())), buffer.size());
			}
		}

		template<class CharT, class SizeT>
		inline auto _data_persistence(CharT * s, SizeT count)
		{
			using value_type = typename std::remove_cv_t<std::remove_reference_t<CharT>>;
			return std::basic_string<value_type>(s, count);
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffer& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer&& data)
		{
			return std::move(data);
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffer& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer&& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(std::move(data)));
		}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffers_1& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1&& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(std::move(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffers_1& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1&& data)
		{
			return static_cast<derived_t&>(*this)._data_persistence(asio::const_buffer(std::move(data)));
		}
#endif

	protected:
	};
}

#endif // !__ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__
