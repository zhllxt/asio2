/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__
#define __ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <memory>
#include <utility>
#include <string>
#include <string_view>

#include <asio2/base/iopool.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>
#include <asio2/base/detail/buffer_wrap.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class data_persistence_cp
	{
	public:
		/**
		 * @brief constructor
		 */
		data_persistence_cp() = default;

		/**
		 * @brief destructor
		 */
		~data_persistence_cp() = default;

	protected:
		template<class T>
		inline auto _data_persistence(T&& data)
		{
			using data_type = detail::remove_cvref_t<T>;
			
			derived_t& derive = static_cast<derived_t&>(*this);

			// std::string_view
			if constexpr /**/ (detail::is_string_view_v<data_type>)
			{
				using value_type = typename data_type::value_type;
				return detail::call_data_filter_before_send(derive,
					std::basic_string<value_type>(data.data(), data.size()));
			}
			// char* , const char* , const char* const
			else if constexpr (detail::is_char_pointer_v<data_type>)
			{
				using value_type = typename detail::remove_cvref_t<std::remove_pointer_t<data_type>>;
				return detail::call_data_filter_before_send(derive,
					std::basic_string<value_type>(std::forward<T>(data)));
			}
			// char[]
			else if constexpr (detail::is_char_array_v<data_type>)
			{
				using value_type = typename detail::remove_cvref_t<std::remove_all_extents_t<data_type>>;
				return detail::call_data_filter_before_send(derive,
					std::basic_string<value_type>(std::forward<T>(data)));
			}
			// object like : std::string, std::vector
			else if constexpr (
				std::is_move_constructible_v          <data_type> ||
				std::is_trivially_move_constructible_v<data_type> ||
				std::is_nothrow_move_constructible_v  <data_type> ||
				std::is_move_assignable_v             <data_type> ||
				std::is_trivially_move_assignable_v   <data_type> ||
				std::is_nothrow_move_assignable_v     <data_type> )
			{
				return detail::call_data_filter_before_send(derive, std::forward<T>(data));
			}
			else
			{
				// PodType (&data)[N] like : int buf[5], double buf[5]
				auto buffer = asio::buffer(data);
				return detail::call_data_filter_before_send(derive,
					std::string(reinterpret_cast<const std::string::value_type*>(
					const_cast<const void*>(buffer.data())), buffer.size()));
			}
		}

		template<class CharT, class SizeT>
		inline auto _data_persistence(CharT * s, SizeT count)
		{
			using value_type = typename detail::remove_cvref_t<CharT>;
			
			derived_t& derive = static_cast<derived_t&>(*this);

			if (!s)
			{
				return std::basic_string<value_type>{};
			}

			return detail::call_data_filter_before_send(derive,
				std::basic_string<value_type>(s, std::size_t(count)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffer& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffer&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			// "data" is rvalue reference, should use std::move()
			return detail::call_data_filter_before_send(derive, std::move(data));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffer& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffer&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(std::move(data))));
		}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::const_buffers_1& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::const_buffers_1&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(std::move(data))));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(const asio::mutable_buffers_1& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(data)));
		}

		template<typename = void>
		inline auto _data_persistence(asio::mutable_buffers_1&& data) noexcept
		{
			derived_t& derive = static_cast<derived_t&>(*this);

			return detail::call_data_filter_before_send(derive,
				derive._data_persistence(asio::const_buffer(std::move(data))));
		}
#endif

	protected:
	};
}

#endif // !__ASIO2_DATA_PERSISTENCE_COMPONENT_HPP__
