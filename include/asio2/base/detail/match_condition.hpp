/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_MATCH_CONDITION_HPP__
#define __ASIO2_MATCH_CONDITION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>

#include <string>
#include <string_view>
#include <utility>

#include <asio2/external/asio.hpp>
#include <asio2/base/detail/util.hpp>

namespace asio2::detail
{
	namespace
	{
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		using diff_type = typename iterator::difference_type;

		std::pair<iterator, bool> dgram_match_role(iterator begin, iterator end) noexcept
		{
			for (iterator p = begin; p < end;)
			{
				// If 0~253, current byte are the payload length.
				if (std::uint8_t(*p) < std::uint8_t(254))
				{
					std::uint8_t payload_size = static_cast<std::uint8_t>(*p);

					++p;

					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
				}

				// If 254, the following 2 bytes interpreted as a 16-bit unsigned integer
				// are the payload length.
				if (std::uint8_t(*p) == std::uint8_t(254))
				{
					++p;

					if (end - p < 2)
						break;

					std::uint16_t payload_size = *(reinterpret_cast<const std::uint16_t*>(p.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(
							std::addressof(payload_size)));
					}

					// illegal data
					if (payload_size < static_cast<std::uint16_t>(254))
						return std::pair(begin, true);

					p += 2;
					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
				}

				// If 255, the following 8 bytes interpreted as a 64-bit unsigned integer
				// (the most significant bit MUST be 0) are the payload length.
				if (std::uint8_t(*p) == 255)
				{
					++p;

					if (end - p < 8)
						break;

					// the most significant bit MUST be 0
					std::int64_t payload_size = *(reinterpret_cast<const std::int64_t*>(p.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::int64_t)>(reinterpret_cast<std::uint8_t*>(
							std::addressof(payload_size)));
					}

					// illegal data
					if (payload_size <= static_cast<std::int64_t>((std::numeric_limits<std::uint16_t>::max)()))
						return std::pair(begin, true);

					p += 8;
					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
				}

				ASIO2_ASSERT(false);
			}
			return std::pair(begin, false);
		}

		/*
		std::pair<iterator, bool> json_format_match_role_impl(
			iterator begin, iterator end,
			typename iterator::value_type head, typename iterator::value_type tail) noexcept
		{
			for (iterator p = begin; p < end; ++p)
			{
				if (std::isspace(*p))
					continue;

				// illegal data
				if (*p != head)
					return std::pair(begin, true);

				for (std::int32_t depth = 0; p < end; ++p)
				{
					if (*p == head)
					{
						++depth;
					}
					else if (*p == tail)
					{
						--depth;

						if (depth == 0)
							return std::pair(p + 1, true);
					}
				}
			}
			return std::pair(begin, false);
		}

		std::pair<iterator, bool> json_object_match_role(iterator begin, iterator end) noexcept
		{
			return json_format_match_role_impl(begin, end, '{', '}');
		}

		std::pair<iterator, bool> json_array_match_role(iterator begin, iterator end) noexcept
		{
			return json_format_match_role_impl(begin, end, '[', ']');
		}

		std::pair<iterator, bool> json_match_role(iterator begin, iterator end) noexcept
		{
			for (iterator p = begin; p < end; ++p)
			{
				if (std::isspace(*p))
					continue;

				// object
				if (*p == '{')
					return json_object_match_role(begin, end);

				// array
				if (*p == '[')
					return json_array_match_role(begin, end);

				// string
				if (*p == '\"')
				{
					for (++p; p < end; ++p)
					{
						if (*p == '\"')
							return std::pair(p + 1, true);
					}
					break;
				}

				// null
				if (*p == 'n')
				{
					if (end - p >= static_cast<diff_type>(4))
					{
						if (p[1] == 'u' && p[2] == 'l' && p[3] == 'l')
							return std::pair(p + 4, true);
						else
							return std::pair(begin, true);
					}

					break;
				}

				// integer
				// we can't know when the numeric string will end, so we can only return the entire string.
				if ((*p >= '0') && (*p <= '9'))
					return std::pair(end, true);

				// illegal data
				return std::pair(begin, true);
			}
			return std::pair(begin, false);
		}
		*/
	}
}

namespace asio2::detail
{
	//struct use_sync_t {};
	struct use_kcp_t {};
	struct use_dgram_t {};
	struct hook_buffer_t {};
}

namespace asio2::detail
{
	/**
	 * use condition_t to avoid having the same name as make_error_condition(condition c)
	 */
	template<class T>
	class condition_t
	{
	public:
		using type = T;

		// must use explicit, Otherwise, there will be an error when there are the following
		// statements: condition_t<char> c1; auto c2 = c1;
		template<class C, std::enable_if_t<!std::is_base_of_v<condition_t, detail::remove_cvref_t<C>>, int> = 0>
		explicit condition_t(C c) noexcept : c_(std::move(c)) {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ this->c_ }; }

		inline T& operator()() noexcept { return this->c_; }
		inline T& lowest    () noexcept { return this->c_; }

	protected:
		T c_;
	};

	// C++17 class template argument deduction guides
	template<class T>
	condition_t(T)->condition_t<std::remove_reference_t<T>>;

	template<>
	class condition_t<void>
	{
	public:
		using type = void;

		 condition_t() noexcept = default;
		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{}; }

		inline void operator()() noexcept {}
		inline void lowest    () noexcept {}
	};

	template<>
	class condition_t<use_dgram_t>
	{
	public:
		using type = use_dgram_t;

		explicit condition_t(use_dgram_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ use_dgram_t{} }; }

		inline auto& operator()() noexcept { return dgram_match_role; }
		inline auto& lowest    () noexcept { return dgram_match_role; }
	};

	template<>
	class condition_t<use_kcp_t>
	{
	public:
		using type = use_kcp_t;

		explicit condition_t(use_kcp_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ use_kcp_t{} }; }

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }
		inline asio::detail::transfer_at_least_t lowest    () noexcept { return asio::transfer_at_least(1); }
	};

	template<>
	class condition_t<hook_buffer_t>
	{
	public:
		using type = hook_buffer_t;

		explicit condition_t(hook_buffer_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ hook_buffer_t{} }; }

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }
		inline asio::detail::transfer_at_least_t lowest    () noexcept { return asio::transfer_at_least(1); }
	};
}

namespace asio2
{
	//constexpr static detail::use_sync_t    use_sync;

	// https://github.com/skywind3000/kcp
	constexpr static detail::use_kcp_t     use_kcp;

	constexpr static detail::use_dgram_t   use_dgram;

	constexpr static detail::hook_buffer_t hook_buffer;
}

#endif // !__ASIO2_MATCH_CONDITION_HPP__
