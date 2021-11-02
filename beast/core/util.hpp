//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_ASSERT_HPP
#define BEAST_ASSERT_HPP

#if !defined(NDEBUG) && !defined(_DEBUG) && !defined(DEBUG)
#define NDEBUG
#endif

#include <cassert>

namespace beast {

	#ifdef BEAST_ASSERT
		static_assert(false, "Unknown BEAST_ASSERT definition will affect the relevant functions of this program.");
	#else
		#if defined(_DEBUG) || defined(DEBUG)
			#define BEAST_ASSERT(expr) assert(expr);
			#define BEAST_ASSERT_MSG(expr, msg) assert((expr)&&(msg))
		#else
			#define BEAST_ASSERT(expr) ((void)0);
			#define BEAST_ASSERT_MSG(expr, msg) ((void)0);
		#endif
	#endif

	#define BEAST_VERIFY(expr) BEAST_ASSERT(expr)
	#define BEAST_VERIFY_MSG(expr, msg) BEAST_ASSERT_MSG(expr,msg)

	#define BEAST_THROW_EXCEPTION(e) { throw e; }
	#define BEAST_STATIC_ASSERT(...) static_assert(__VA_ARGS__, #__VA_ARGS__)

	#ifndef BEAST_FALLTHROUGH
	#  define BEAST_FALLTHROUGH ((void)0)
	#endif

	#if !defined(BEAST_LIKELY)
	#  define BEAST_LIKELY(x) x
	#endif
	#if !defined(BEAST_UNLIKELY)
	#  define BEAST_UNLIKELY(x) x
	#endif

	#if !defined(BEAST_ALIGNMENT)
	#  define BEAST_ALIGNMENT(x) alignas(x)
	#endif

	#define BEAST_STRINGIZE(X) BEAST_DO_STRINGIZE(X)
	#define BEAST_DO_STRINGIZE(X) #X

	template <typename... Ts>
	inline constexpr void ignore_unused(Ts const& ...)
	{}

	template <typename... Ts>
	inline constexpr void ignore_unused()
	{}

	namespace detail
	{
		//! Returns true if the current machine is little endian
		/*! @ingroup Internal */
		inline std::uint8_t is_little_endian()
		{
			static std::int32_t test = 1;
			return *reinterpret_cast<std::int8_t*>(&test) == 1;
		}

		//! Swaps the order of bytes for some chunk of memory
		/*! @param data The data as a uint8_t pointer
			@tparam DataSize The true size of the data
			@ingroup Internal */
		template <std::size_t DataSize>
		inline void swap_bytes(std::uint8_t * data)
		{
			for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
				std::swap(data[i], data[DataSize - i - 1]);
		}
	}

	namespace endian
	{
		template <class EndianReversible>
		inline EndianReversible native_to_big(EndianReversible x) noexcept
		{
			BEAST_STATIC_ASSERT(std::is_integral_v<EndianReversible>);
			if (detail::is_little_endian())
				detail::swap_bytes<sizeof(EndianReversible)>(reinterpret_cast<std::uint8_t *>(&x));
			return x;
		}

		template <class EndianReversible>
		inline EndianReversible native_to_little(EndianReversible x) noexcept
		{
			BEAST_STATIC_ASSERT(std::is_integral_v<EndianReversible>);
			if (!detail::is_little_endian())
				detail::swap_bytes<sizeof(EndianReversible)>(reinterpret_cast<std::uint8_t *>(&x));
			return x;
		}

		template <class EndianReversible>
		inline EndianReversible big_to_native(EndianReversible x) noexcept
		{
			BEAST_STATIC_ASSERT(std::is_integral_v<EndianReversible>);
			if (detail::is_little_endian())
				detail::swap_bytes<sizeof(EndianReversible)>(reinterpret_cast<std::uint8_t *>(&x));
			return x;
		}

		template <class EndianReversible>
		inline EndianReversible little_to_native(EndianReversible x) noexcept
		{
			BEAST_STATIC_ASSERT(std::is_integral_v<EndianReversible>);
			if (!detail::is_little_endian())
				detail::swap_bytes<sizeof(EndianReversible)>(reinterpret_cast<std::uint8_t *>(&x));
			return x;
		}
	}

} // beast

#endif
