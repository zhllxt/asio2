/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
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
#include <type_traits>

#include <asio2/base/selector.hpp>
#include <asio2/base/iopool.hpp>

namespace asio2::detail
{
	enum class state_t : std::int8_t { stopped, stopping, starting, started };

	static long constexpr  tcp_handshake_timeout = 5 * 1000;
	static long constexpr  udp_handshake_timeout = 5 * 1000;
	static long constexpr http_handshake_timeout = 5 * 1000;

	static long constexpr  tcp_connect_timeout = 5 * 1000;
	static long constexpr  udp_connect_timeout = 5 * 1000;
	static long constexpr http_connect_timeout = 5 * 1000;

	static long constexpr  tcp_silence_timeout = 60 * 60 * 1000;
	static long constexpr  udp_silence_timeout = 60 * 1000;
	static long constexpr http_silence_timeout = 85 * 1000;

	static long constexpr http_execute_timeout = 3 * 1000;

	static long constexpr ssl_shutdown_timeout = 1500;
	static long constexpr  ws_shutdown_timeout = 1500;

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
	inline std::string to_string(char * s)
	{
		return (s ? std::string(s) : std::string{});
	}
	inline std::string to_string(const char * s)
	{
		return (s ? std::string(s) : std::string{});
	}
	inline std::string to_string(const std::string_view& sv)
	{
		return std::string(sv);
	}
	inline std::string to_string(std::string_view&& sv)
	{
		return std::string(sv);
	}
	inline std::string to_string(const std::string& s)
	{
		return s;
	}
	inline std::string to_string(std::string&& s)
	{
		return std::move(s);
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
		template<class ...Args> ignore(Args&&...) {}

		template<class T>
		constexpr const ignore& operator=(const T&) const noexcept	// strengthened
		{
			return (*this);
		}

		template<class ...Args>
		static inline constexpr const void unused(Args&&...) noexcept {}
	};

	template<class Rep, class Period, class Fn>
	std::shared_ptr<asio::steady_timer> mktimer(io_t& io, std::chrono::duration<Rep, Period> duration, Fn&& fn)
	{
		std::shared_ptr<asio::steady_timer> timer =
			std::make_shared<asio::steady_timer>(io.context());
		auto post = std::make_shared<std::unique_ptr<std::function<void()>>>();
		*post = std::make_unique<std::function<void()>>([&io, duration, f = std::forward<Fn>(fn), timer, post]()
		{
			timer->expires_after(duration);
			timer->async_wait(asio::bind_executor(io.strand(), [&f, &post](const error_code & ec) mutable
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


	template<typename, typename = void>
	struct is_string : std::false_type {};

	template<typename T>
	struct is_string<T, std::void_t<typename T::value_type, typename T::traits_type, typename T::allocator_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string<typename T::value_type, typename T::traits_type, typename T::allocator_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_string_v = is_string<std::remove_cv_t<std::remove_reference_t<T>>>::value;


	template<typename, typename = void>
	struct is_string_view : std::false_type {};

	template<typename T>
	struct is_string_view<T, std::void_t<typename T::value_type, typename T::traits_type,
		typename std::enable_if_t<std::is_same_v<T,
		std::basic_string_view<typename T::value_type, typename T::traits_type>>>>> : std::true_type {};

	template<class T>
	inline constexpr bool is_string_view_v = is_string_view<std::remove_cv_t<std::remove_reference_t<T>>>::value;


	template<typename String>
	inline std::string to_string_host(String&& host)
	{
		using type = std::remove_cv_t<std::remove_reference_t<String>>;
		std::string h;
		if constexpr (is_string_view_v<type>)
		{
			h = { host.data(),host.size() };
		}
		else if constexpr (std::is_pointer_v<type>)
		{
			if (host) h = host;
		}
		else
		{
			h = std::forward<String>(host);
		}
		return h;
	}

	template<typename StringOrInt>
	inline std::string to_string_port(StringOrInt&& port)
	{
		using type = std::remove_cv_t<std::remove_reference_t<StringOrInt>>;
		std::string p;
		if constexpr (is_string_view_v<type>)
		{
			p = { port.data(),port.size() };
		}
		else if constexpr (std::is_integral_v<type>)
		{
			p = std::to_string(port);
		}
		else if constexpr (std::is_pointer_v<type>)
		{
			if (port) p = port;
		}
		else
		{
			p = std::forward<StringOrInt>(port);
		}
		return p;
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
