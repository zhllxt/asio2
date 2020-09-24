/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_UTIL_HPP__
#define __ASIO2_UTIL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <cstdarg>
#include <cstdio>
#include <cwchar>
#include <climits>
#include <cctype>

#include <string>
#include <string_view>
#include <type_traits>
#include <memory>
#include <future>
#include <functional>
#include <tuple>
#include <atomic>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>
#include <asio2/base/error.hpp>

namespace asio2::detail
{
	enum class state_t : std::int8_t { stopped, stopping, starting, started };

	static long constexpr  tcp_handshake_timeout = 5 * 1000;
	static long constexpr  udp_handshake_timeout = 5 * 1000;
	static long constexpr http_handshake_timeout = 5 * 1000;

	static long constexpr  tcp_connect_timeout   = 5 * 1000;
	static long constexpr  udp_connect_timeout   = 5 * 1000;
	static long constexpr http_connect_timeout   = 5 * 1000;

	static long constexpr  tcp_silence_timeout   = 60 * 60 * 1000;
	static long constexpr  udp_silence_timeout   = 60 * 1000;
	static long constexpr http_silence_timeout   = 85 * 1000;

	static long constexpr http_execute_timeout   = 5 * 1000;

	static long constexpr ssl_shutdown_timeout   = 5 * 1000;
	static long constexpr  ws_shutdown_timeout   = 5 * 1000;

	static long constexpr ssl_handshake_timeout  = 5 * 1000;
	static long constexpr  ws_handshake_timeout  = 5 * 1000;

	/*
	 * The read buffer has to be at least as large
	 * as the largest possible control frame including
	 * the frame header.
	 * refrenced from beast stream.hpp
	 */
	static std::size_t constexpr  tcp_frame_size = 1536;
	static std::size_t constexpr  udp_frame_size = 1024;
	static std::size_t constexpr http_frame_size = 1536;
}

namespace asio2::detail
{
	template <typename Enumeration>
	inline constexpr auto enum_to_int(Enumeration const value) ->
		typename std::underlying_type<Enumeration>::type
	{
		return static_cast<typename std::underlying_type<Enumeration>::type>(value);
	}

	/**
	 * BKDR Hash Function
	 */
	inline std::size_t bkdr_hash(const unsigned char * const p, std::size_t size)
	{
		std::size_t v = 0;
		for (std::size_t i = 0; i < size; ++i)
		{
			v = v * 131 + static_cast<std::size_t>(p[i]);
		}
		return v;
	}

	/**
	 * Fnv1a Hash Function
	 * Reference from Visual c++ implementation, see vc++ std::hash
	 */
	template<typename T>
	inline T fnv1a_hash(const unsigned char * const p, const T size) noexcept
	{
		static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Must be 32 or 64 digits");
		T v;
		if constexpr (sizeof(T) == 4)
			v = 2166136261u;
		else
			v = 14695981039346656037ull;
		for (T i = 0; i < size; ++i)
		{
			v ^= static_cast<T>(p[i]);
			if constexpr (sizeof(T) == 4)
				v *= 16777619u;
			else
				v *= 1099511628211ull;
		}
		return (v);
	}

	// struct that ignores assignments
	struct ignore
	{
		template<class ...Args> ignore(const Args&...) {}

		template<class T>
		constexpr const ignore& operator=(const T&) const noexcept	// strengthened
		{
			return (*this);
		}

		template<class ...Args>
		static inline constexpr const void unused(const Args&...) noexcept {}
	};

	template <typename... Ts>
	inline constexpr void ignore_unused(Ts const& ...) {}

	template <typename... Ts>
	inline constexpr void ignore_unused() {}

	template<class T>
	class copyable_wrapper
	{
	public:
		using value_type = T;

		template<typename ...Args>
		copyable_wrapper(Args&&... args) : raw(std::forward<Args>(args)...) { }
		template<typename = void>
		copyable_wrapper(T&& o) : raw(std::move(o)) { }

		copyable_wrapper(copyable_wrapper&&) = default;
		copyable_wrapper& operator=(copyable_wrapper&&) = default;

		copyable_wrapper(copyable_wrapper const& r) : raw(const_cast<T&&>(r.raw)) { throw 0; }
		copyable_wrapper& operator=(copyable_wrapper const& r) { raw = const_cast<T&&>(r.raw); throw 0; }

		T& operator()() noexcept { return raw; }

	protected:
		T raw;
	};

	template<typename, typename = void>
	struct is_copyable_wrapper : std::false_type {};

	template<typename T>
	struct is_copyable_wrapper<T, std::void_t<typename T::value_type,
		typename std::enable_if_t<std::is_same_v<T,
		copyable_wrapper<typename T::value_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_copyable_wrapper_v = is_copyable_wrapper<T>::value;


	template<class Rep, class Period, class Fn>
	std::shared_ptr<asio::steady_timer> mktimer(io_t& io, std::chrono::duration<Rep, Period> duration, Fn&& fn)
	{
		std::shared_ptr<asio::steady_timer> timer =
			std::make_shared<asio::steady_timer>(io.context());
		auto post = std::make_shared<std::unique_ptr<std::function<void()>>>();
		*post = std::make_unique<std::function<void()>>(
			[&io, duration, f = std::forward<Fn>(fn), timer, post]() mutable
		{
			timer->expires_after(duration);
			timer->async_wait(asio::bind_executor(io.strand(), [&f, &post]
			(const error_code & ec) mutable
			{
				if (f(ec))
					(**post)();
				else
					(*post).reset();
			}));
		});
		(**post)();
		return timer;
	}

	template<class T, bool isIntegral = true, bool isUnsigned = true, bool SkipZero = true>
	class id_maker
	{
	public:
		id_maker(T init = static_cast<T>(1)) : id(init)
		{
			if constexpr (isIntegral)
			{
				static_assert(std::is_integral_v<T>, "T must be integral");
				if constexpr (isUnsigned)
				{
					static_assert(std::is_unsigned_v<T>, "T must be unsigned integral");
				}
				else
				{
					static_assert(true);
				}
			}
			else
			{
				static_assert(true);
			}
		}
		inline T mkid()
		{
			if constexpr (SkipZero)
			{
				T r = id.fetch_add(static_cast<T>(1));
				return (r == 0 ? id.fetch_add(static_cast<T>(1)) : r);
			}
			else
			{
				return id.fetch_add(static_cast<T>(1));
			}
		}
	protected:
		std::atomic<T> id;
	};


	// example : static_assert(is_template_instantiable_v<std::vector, double>);
	//           static_assert(is_template_instantiable_v<std::optional, int, int>);
	template<template<typename...> typename T, typename AlwaysVoid, typename... Args>
	struct is_template_instantiable : std::false_type {};

	template<template<typename...> typename T, typename... Args>
	struct is_template_instantiable<T, std::void_t<T<Args...>>, Args...> : std::true_type {};

	template<template<typename...> typename T, typename... Args>
	inline constexpr bool is_template_instantiable_v = is_template_instantiable<T, void, Args...>::value;


	// example : static_assert(is_template_instance_of<std::vector, std::vector<int>>);
	template<template<typename...> class U, typename T>
	struct is_template_instance_of : std::false_type {};

	template<template<typename...> class U, typename...Args>
	struct is_template_instance_of<U, U<Args...>> : std::true_type {};

	template<template<typename...> class U, typename...Args>
	inline constexpr bool is_template_instance_of_v = is_template_instance_of<U, Args...>::value;

	template<typename T> struct is_tuple : is_template_instance_of<std::tuple, T> {};


	template<typename, typename = void>
	struct is_string : std::false_type {};

	template<typename T>
	struct is_string<T, std::void_t<typename T::value_type, typename T::traits_type, typename T::allocator_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>>>>
		: std::true_type {};

	template<class T>
	inline constexpr bool is_string_v = is_string<T>::value;


	template<typename, typename = void>
	struct is_string_view : std::false_type {};

	template<typename T>
	struct is_string_view<T, std::void_t<typename T::value_type, typename T::traits_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string_view<typename T::value_type, typename T::traits_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_string_view_v = is_string_view<T>::value;


	template<typename T>
	inline std::string to_string(T&& v)
	{
		using type = std::remove_cv_t<std::remove_reference_t<T>>;
		std::string s;
		if constexpr (is_string_view_v<type>)
		{
			s = { v.data(),v.size() };
		}
		else if constexpr (std::is_integral_v<type>)
		{
			s = std::to_string(v);
		}
		else if constexpr (std::is_pointer_v<type>)
		{
			if (v) s = v;
		}
		else if constexpr (std::is_array_v<type>)
		{
			s = std::forward<T>(v);
		}
		else
		{
			s = std::forward<T>(v);
		}
		return s;
	}

	template<typename IntegerType, typename T>
	inline IntegerType to_integer(T&& v)
	{
		using type = std::remove_cv_t<std::remove_reference_t<T>>;
		if constexpr (std::is_integral_v<type>)
			return static_cast<IntegerType>(v);
		else
			return static_cast<IntegerType>(std::stoull(to_string(std::forward<T>(v))));
	}

	template<typename Protocol, typename String, typename StrOrInt>
	inline Protocol to_endpoint(String&& host, StrOrInt&& port)
	{
		std::string h = to_string(std::forward<String>(host));
		std::string p = to_string(std::forward<StrOrInt>(port));

		asio::io_context ioc;
		// the resolve function is a time-consuming operation
		if /**/ constexpr (std::is_same_v<asio::ip::udp::endpoint, Protocol>)
		{
			asio::ip::udp::resolver resolver(ioc);
			asio::ip::udp::endpoint endpoint = *resolver.resolve(h, p,
				asio::ip::resolver_base::flags::address_configured);
			return endpoint;
		}
		else if constexpr (std::is_same_v<asio::ip::tcp::endpoint, Protocol>)
		{
			asio::ip::tcp::resolver resolver(ioc);
			asio::ip::tcp::endpoint endpoint = *resolver.resolve(h, p,
				asio::ip::resolver_base::flags::address_configured);
			return endpoint;
		}
		else
		{
			ASIO2_ASSERT(false);
		}
	}

	// Returns true if the current machine is little endian
	inline bool is_little_endian()
	{
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(&test) == 1);
	}

	/**
	 * Swaps the order of bytes for some chunk of memory
	 * @param data The data as a uint8_t pointer
	 * @tparam DataSize The true size of the data
	 */
	template <std::size_t DataSize>
	inline void swap_bytes(std::uint8_t * data)
	{
		for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
			std::swap(data[i], data[DataSize - i - 1]);
	}
}

// custom specialization of std::hash can be injected in namespace std
namespace std
{
	template<> struct hash<asio::ip::udp::endpoint>
	{
		typedef asio::ip::udp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return asio2::detail::bkdr_hash((const unsigned char *)(&s), sizeof(argument_type));
		}
	};

	template<> struct hash<asio::ip::tcp::endpoint>
	{
		typedef asio::ip::tcp::endpoint argument_type;
		typedef std::size_t result_type;
		inline result_type operator()(argument_type const& s) const noexcept
		{
			//return std::hash<std::string_view>()(std::string_view{
			//	reinterpret_cast<std::string_view::const_pointer>(&s),sizeof(argument_type) });
			return asio2::detail::bkdr_hash((const unsigned char *)(&s), sizeof(argument_type));
		}
	};
}

#endif // !__ASIO2_UTIL_HPP__
