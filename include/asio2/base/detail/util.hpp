/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <utility>
#include <atomic>
#include <limits>
#include <thread>
#include <mutex>

#include <asio2/base/error.hpp>

#include <asio2/base/detail/filesystem.hpp>
#include <asio2/base/detail/shared_mutex.hpp>
#include <asio2/base/detail/type_traits.hpp>

#include <asio2/util/string.hpp>

// 
// you can define this macro before include asio2 files to use custom ssl method.
// eg:
// #define ASIO2_DEFAULT_SSL_METHOD asio::ssl::context::tlsv13
// #include <asio2/asio2.hpp>
// 
#ifndef ASIO2_DEFAULT_SSL_METHOD
#define ASIO2_DEFAULT_SSL_METHOD asio::ssl::context::sslv23
#endif

namespace asio2
{
	template<typename = void>
	inline std::string to_string(const asio::const_buffer& v) noexcept
	{
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string to_string(const asio::mutable_buffer& v) noexcept
	{
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
	template<typename = void>
	inline std::string to_string(const asio::const_buffers_1& v) noexcept
	{
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string to_string(const asio::mutable_buffers_1& v) noexcept
	{
		return std::string{ (std::string::pointer)(v.data()), v.size() };
	}
#endif

	template<typename = void>
	inline std::string_view to_string_view(const asio::const_buffer& v) noexcept
	{
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string_view to_string_view(const asio::mutable_buffer& v) noexcept
	{
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

#if !defined(ASIO_NO_DEPRECATED) && !defined(BOOST_ASIO_NO_DEPRECATED)
	template<typename = void>
	inline std::string_view to_string_view(const asio::const_buffers_1& v) noexcept
	{
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}

	template<typename = void>
	inline std::string_view to_string_view(const asio::mutable_buffers_1& v) noexcept
	{
		return std::string_view{ (std::string_view::const_pointer)(v.data()), v.size() };
	}
#endif
}

namespace asio2::detail
{
	using asio2::to_string;
	using asio2::to_string_view;
	using asio2::to_numeric;
}

namespace asio2::detail
{
	struct tcp_tag  { using tl_tag_type = tcp_tag ; }; // transport layer
	struct udp_tag  { using tl_tag_type = udp_tag ; }; // transport layer
	struct cast_tag { using tl_tag_type = cast_tag; };

	struct ssl_stream_tag {};
	struct ws_stream_tag  {};
}

namespace asio2
{
	template<typename Protocol, typename String, typename StrOrInt>
	inline Protocol to_endpoint(String&& host, StrOrInt&& port)
	{
		std::string h = detail::to_string(std::forward<String>(host));
		std::string p = detail::to_string(std::forward<StrOrInt>(port));

		asio::io_context ioc;
		// the resolve function is a time-consuming operation
		if /**/ constexpr (std::is_same_v<asio::ip::udp::endpoint, Protocol>)
		{
			error_code ec;
			asio::ip::udp::resolver resolver(ioc);
			auto rs = resolver.resolve(h, p, asio::ip::resolver_base::flags::address_configured, ec);
			if (ec || rs.empty())
			{
				set_last_error(ec ? ec : asio::error::host_not_found);
				return *rs;
			}
			else
			{
				clear_last_error();
				return asio::ip::udp::endpoint{};
			}
		}
		else if constexpr (std::is_same_v<asio::ip::tcp::endpoint, Protocol>)
		{
			error_code ec;
			asio::ip::tcp::resolver resolver(ioc);
			auto rs = resolver.resolve(h, p, asio::ip::resolver_base::flags::address_configured, ec);
			if (ec || rs.empty())
			{
				set_last_error(ec ? ec : asio::error::host_not_found);
				return *rs;
			}
			else
			{
				clear_last_error();
				return asio::ip::tcp::endpoint{};
			}
		}
		else
		{
			static_assert(detail::always_false_v<Protocol>);
		}
	}
}

namespace asio2::detail
{
	using asio2::to_endpoint;
}

namespace asio2::detail
{
	enum class state_t : std::int8_t { stopped, stopping, starting, started };

	template<typename = void>
	inline constexpr std::string_view to_string(state_t v)
	{
		using namespace std::string_view_literals;
		switch (v)
		{
		case state_t::stopped  : return "stopped";
		case state_t::stopping : return "stopping";
		case state_t::starting : return "starting";
		case state_t::started  : return "started";
		default                : return "none";
		}
		return "none";
	}

	// /bho/beast/websocket/stream_base.hpp line 147
	// opt.handshake_timeout = std::chrono::seconds(30);

	// When there are a lot of connections, there will maybe a lot of COSE_WAIT,LAST_ACK,TIME_WAIT
	// and other problems, resulting in the client being unable to connect to the server normally.
	// Increasing the connect,handshake,shutdown timeout can effectively alleviate this problem.

	static long constexpr  tcp_handshake_timeout = 30 * 1000;
	static long constexpr  udp_handshake_timeout = 30 * 1000;
	static long constexpr http_handshake_timeout = 30 * 1000;

	static long constexpr  tcp_connect_timeout   = 30 * 1000;
	static long constexpr  udp_connect_timeout   = 30 * 1000;
	static long constexpr http_connect_timeout   = 30 * 1000;

	static long constexpr  tcp_silence_timeout   = 60 * 60 * 1000;
	static long constexpr  udp_silence_timeout   = 60 * 1000;
	static long constexpr http_silence_timeout   = 85 * 1000;
	static long constexpr mqtt_silence_timeout   = 90 * 1000; // 60 * 1.5

	static long constexpr http_execute_timeout   = 15 * 1000;
	static long constexpr icmp_execute_timeout   =  4 * 1000;

	static long constexpr ssl_shutdown_timeout   = 30 * 1000;
	static long constexpr  ws_shutdown_timeout   = 30 * 1000;

	static long constexpr ssl_handshake_timeout  = 30 * 1000;
	static long constexpr  ws_handshake_timeout  = 30 * 1000;

	/*
	 * The read buffer has to be at least as large
	 * as the largest possible control frame including
	 * the frame header.
	 * refrenced from beast stream.hpp
	 */
	// udp MTU : https://zhuanlan.zhihu.com/p/301276548
	static std::size_t constexpr  tcp_frame_size = 1536;
	static std::size_t constexpr  udp_frame_size = 1024;
	static std::size_t constexpr http_frame_size = 1536;

	static std::size_t constexpr max_buffer_size = (std::numeric_limits<std::size_t>::max)();

	// std::thread::hardware_concurrency() is not constexpr, so use it with function form
	// @see: asio::detail::default_thread_pool_size()
	template<typename = void>
	inline std::size_t default_concurrency() noexcept
	{
		std::size_t num_threads = std::thread::hardware_concurrency() * 2;
		num_threads = num_threads == 0 ? 2 : num_threads;
		return num_threads;
	}
}

namespace asio2::detail
{
	/**
	 * BKDR Hash Function
	 */
	template<typename = void>
	inline std::size_t bkdr_hash(const unsigned char * const p, std::size_t size) noexcept
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

	template<typename T>
	inline T fnv1a_hash(T v, const unsigned char * const p, const T size) noexcept
	{
		static_assert(sizeof(T) == 4 || sizeof(T) == 8, "Must be 32 or 64 digits");
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


	template<class T>
	class copyable_wrapper
	{
	public:
		using value_type = T;

		template<typename ...Args>
		copyable_wrapper(Args&&... args) noexcept : raw(std::forward<Args>(args)...) { }
		template<typename = void>
		copyable_wrapper(T&& o) noexcept : raw(std::move(o)) { }

		copyable_wrapper(copyable_wrapper&&) noexcept = default;
		copyable_wrapper& operator=(copyable_wrapper&&) noexcept = default;

		copyable_wrapper(copyable_wrapper const& r) noexcept : raw(const_cast<T&&>(r.raw)) { }
		copyable_wrapper& operator=(copyable_wrapper const& r) noexcept { raw = const_cast<T&&>(r.raw); }

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


	inline void cancel_timer(asio::steady_timer& timer) noexcept
	{
		try
		{
			timer.cancel();
		}
		catch (system_error const&)
		{
		}
	}

	struct safe_timer
	{
		explicit safe_timer(asio::io_context& ioc) : timer(ioc)
		{
			canceled.clear();
		}

		inline void cancel()
		{
			this->canceled.test_and_set();

			detail::cancel_timer(this->timer);
		}

		/// Timer impl
		asio::steady_timer timer;

		/// Why use this flag, beacuase the ec param maybe zero when the timer callback is
		/// called after the timer cancel function has called already.
		/// Before : need reset the "canceled" flag to false, otherwise after "client.stop();"
		/// then call client.start(...) again, this reconnect timer will doesn't work .
		/// can't put this "clear" code into the timer handle function, beacuse the stop timer
		/// maybe called many times. so, when the "canceled" flag is set false in the timer handle
		/// and the stop timer is called later, then the "canceled" flag will be set true again .
		std::atomic_flag   canceled;
	};

	template<class Rep, class Period, class Fn>
	std::shared_ptr<safe_timer> mktimer(asio::io_context& ioc, std::chrono::duration<Rep, Period> duration, Fn&& fn)
	{
		std::shared_ptr<safe_timer> timer = std::make_shared<safe_timer>(ioc);
		auto post = std::make_shared<std::unique_ptr<std::function<void()>>>();
		*post = std::make_unique<std::function<void()>>(
		[duration, f = std::forward<Fn>(fn), timer, post]() mutable
		{
			timer->timer.expires_after(duration);
			timer->timer.async_wait([&f, &timer, &post](const error_code& ec) mutable
			{
				if (!timer->canceled.test_and_set())
				{
					timer->canceled.clear();

					if (f(ec))
						(**post)();
					else
						(*post).reset();
				}
				else
				{
					(*post).reset();
				}
			});
		});
		(**post)();
		return timer;
	}

	template<class T, bool isIntegral = true, bool isUnsigned = true, bool SkipZero = true>
	class id_maker
	{
	public:
		id_maker(T init = static_cast<T>(1)) noexcept : id(init)
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
		inline T mkid() noexcept
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


	// Returns true if the current machine is little endian
	template<typename = void>
	inline bool is_little_endian() noexcept
	{
		static std::int32_t test = 1;
		return (*reinterpret_cast<std::int8_t*>(std::addressof(test)) == 1);
	}

	/**
	 * Swaps the order of bytes for some chunk of memory
	 * @param data - The data as a uint8_t pointer
	 * @tparam DataSize - The true size of the data
	 */
	template <std::size_t DataSize>
	inline void swap_bytes(std::uint8_t* data) noexcept
	{
		for (std::size_t i = 0, end = DataSize / 2; i < end; ++i)
			std::swap(data[i], data[DataSize - i - 1]);
	}

	/**
	 * Swaps the order of bytes for some chunk of memory
	 * @param v - The variable reference.
	 */
	template <class T>
	inline void swap_bytes(T& v) noexcept
	{
		std::uint8_t* p = reinterpret_cast<std::uint8_t*>(std::addressof(v));
		swap_bytes<sizeof(T)>(p);
	}

	/**
	 * converts the value from host to TCP/IP network byte order (which is big-endian).
	 * @param v - The variable reference.
	 */
	template <class T>
	inline T host_to_network(T v) noexcept
	{
		if (is_little_endian())
		{
			std::uint8_t* p = reinterpret_cast<std::uint8_t*>(std::addressof(v));
			swap_bytes<sizeof(T)>(p);
		}

		return v;
	}

	/**
	 * converts the value from TCP/IP network order to host byte order (which is little-endian on Intel processors).
	 * @param v - The variable reference.
	 */
	template <class T>
	inline T network_to_host(T v) noexcept
	{
		if (is_little_endian())
		{
			std::uint8_t* p = reinterpret_cast<std::uint8_t*>(std::addressof(v));
			swap_bytes<sizeof(T)>(p);
		}

		return v;
	}

	template<class T, class Pointer>
	inline void write(Pointer& p, T v) noexcept
	{
		if constexpr (int(sizeof(T)) > 1)
		{
			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian())
			{
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}

			std::memcpy((void*)p, (const void*)&v, sizeof(T));
		}
		else
		{
			static_assert(sizeof(T) == std::size_t(1));

			*p = std::decay_t<std::remove_pointer_t<detail::remove_cvref_t<Pointer>>>(v);
		}

		p += sizeof(T);
	}

	template<class T, class Pointer>
	inline T read(Pointer& p) noexcept
	{
		T v{};

		if constexpr (int(sizeof(T)) > 1)
		{
			std::memcpy((void*)&v, (const void*)p, sizeof(T));

			// MSDN: The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
			// ** This mean the network byte order is big-endian **
			if (is_little_endian())
			{
				swap_bytes<sizeof(T)>(reinterpret_cast<std::uint8_t *>(std::addressof(v)));
			}
		}
		else
		{
			static_assert(sizeof(T) == std::size_t(1));

			v = T(*p);
		}

		p += sizeof(T);

		return v;
	}

	// C++ SSO : How to programatically find if a std::wstring is allocated with Short String Optimization?
	// https://stackoverflow.com/questions/65736613/c-sso-how-to-programatically-find-if-a-stdwstring-is-allocated-with-short
	template <class T>
	bool is_used_sso(const T& t) noexcept
	{
		using type = typename detail::remove_cvref_t<T>;
		static type st{};
		return t.capacity() == st.capacity();
	}

	template<class T>
	std::size_t sso_buffer_size() noexcept
	{
		using type = typename detail::remove_cvref_t<T>;
		static type st{};
		return st.capacity();
	}

	// Disable std:string's SSO
	// https://stackoverflow.com/questions/34788789/disable-stdstrings-sso
	// std::string str;
	// str.reserve(sizeof(str) + 1);
	template<class String>
	inline void disable_sso(String& str)
	{
		str.reserve(sso_buffer_size<typename detail::remove_cvref_t<String>>() + 1);
	}

	template<class Integer>
	struct integer_add_sub_guard
	{
		 integer_add_sub_guard(Integer& v) noexcept : v_(v) { ++v_; }
		~integer_add_sub_guard()           noexcept         { --v_; }

		Integer& v_;
	};

	// C++17 class template argument deduction guides
	template<class Integer>
	integer_add_sub_guard(Integer&)->integer_add_sub_guard<Integer>;

	template<typename = void>
	bool is_subpath_of(const std::filesystem::path& base, const std::filesystem::path& p) noexcept
	{
		assert(std::filesystem::is_directory(base));

		auto it_base = base.begin();
		auto it_p = p.begin();

		while (it_base != base.end() && it_p != p.end())
		{
			if (*it_base != *it_p)
			{
				return false;
			}

			++it_base;
			++it_p;
		}

		// If all components of base are matched, and p has more components, then base is a subpath of p
		return it_base == base.end() && it_p != p.end();
	}

	template<typename = void>
	std::filesystem::path make_filepath(const std::filesystem::path& base, const std::filesystem::path& p) noexcept
	{
		std::error_code ec{};
		std::filesystem::path b = std::filesystem::canonical(base, ec);
		if (ec)
		{
			return {};
		}

		assert(std::filesystem::is_directory(b));

		std::filesystem::path filepath = b;
		filepath += p;

		filepath = std::filesystem::canonical(filepath, ec);

		if (ec || !is_subpath_of(b, filepath))
		{
			return {};
		}

		return filepath;
	}
}

namespace asio2
{
	enum class net_protocol : std::uint8_t
	{
		none = 0,

		udp = 1,

		tcp,
		http,
		websocket,
		rpc,
		mqtt,

		tcps,
		https,
		websockets,
		rpcs,
		mqtts,

		icmp,
		serialport,

		ws = websocket,
		wss = websockets
	};

	enum class response_mode : std::uint8_t
	{
		automatic = 1,
		manual,
	};
}

// custom specialization of std::hash can be injected in namespace std
// see : struct hash<asio::ip::basic_endpoint<InternetProtocol>> in /asio/ip/basic_endpoint.hpp
#if !defined(ASIO_HAS_STD_HASH)
namespace std
{
	template <>
	struct hash<asio::ip::address_v4>
	{
		std::size_t operator()(const asio::ip::address_v4& addr) const ASIO_NOEXCEPT
		{
			return std::hash<unsigned int>()(addr.to_uint());
		}
	};

	template <>
	struct hash<asio::ip::address_v6>
	{
		std::size_t operator()(const asio::ip::address_v6& addr) const ASIO_NOEXCEPT
		{
			const asio::ip::address_v6::bytes_type bytes = addr.to_bytes();
			std::size_t result = static_cast<std::size_t>(addr.scope_id());
			combine_4_bytes(result, &bytes[0]);
			combine_4_bytes(result, &bytes[4]);
			combine_4_bytes(result, &bytes[8]);
			combine_4_bytes(result, &bytes[12]);
			return result;
		}

	private:
		static void combine_4_bytes(std::size_t& seed, const unsigned char* bytes)
		{
			const std::size_t bytes_hash =
				(static_cast<std::size_t>(bytes[0]) << 24) |
				(static_cast<std::size_t>(bytes[1]) << 16) |
				(static_cast<std::size_t>(bytes[2]) << 8) |
				(static_cast<std::size_t>(bytes[3]));
			seed ^= bytes_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
	};

	template <>
	struct hash<asio::ip::address>
	{
		std::size_t operator()(const asio::ip::address& addr) const ASIO_NOEXCEPT
		{
			return addr.is_v4()
				? std::hash<asio::ip::address_v4>()(addr.to_v4())
				: std::hash<asio::ip::address_v6>()(addr.to_v6());
		}
	};

	template <typename InternetProtocol>
	struct hash<asio::ip::basic_endpoint<InternetProtocol>>
	{
		std::size_t operator()(const asio::ip::basic_endpoint<InternetProtocol>& ep) const ASIO_NOEXCEPT
		{
			std::size_t hash1 = std::hash<asio::ip::address>()(ep.address());
			std::size_t hash2 = std::hash<unsigned short>()(ep.port());
			return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
		}
	};
}
#endif

namespace asio2
{
	namespace detail
	{
		template<class T>
		struct current_object_result_t
		{
			using type = T&;
		};

		template<class T>
		struct current_object_result_t<std::shared_ptr<T>>
		{
			using type = std::weak_ptr<T>&;
		};

		class [[maybe_unused]] external_linkaged_current_object
		{
		public:
			template<class T>
			[[maybe_unused]] static typename current_object_result_t<T>::type get() noexcept
			{
				if constexpr (detail::is_template_instance_of_v<std::shared_ptr, T>)
				{
					thread_local static std::weak_ptr<typename T::element_type> o{};

					return o;
				}
				else
				{
					thread_local static T o{};

					return o;
				}
			}
		};

		namespace internal_linkaged_current_object
		{
			template<class T>
			[[maybe_unused]] static typename current_object_result_t<T>::type get() noexcept
			{
				if constexpr (detail::is_template_instance_of_v<std::shared_ptr, T>)
				{
					thread_local static std::weak_ptr<typename T::element_type> o{};

					return o;
				}
				else
				{
					thread_local static T o{};

					return o;
				}
			}
		}

		template<class T>
		[[maybe_unused]] inline typename current_object_result_t<T>::type get_current_object() noexcept
		{
			return detail::external_linkaged_current_object::get<T>();
		}
	}

	/**
	 * @brief Get the current caller object in the current thread.
	 * @tparam T - If the object is created on the stack such as "asio2::rpc_client client", the T can
	 *             only be asio2::rpc_client& or asio2::rpc_client*
	 *             If the object is created on the heap such as "std::shared_ptr<asio2::rpc_session>", 
	 *             the T can only be std::shared_ptr<asio2::rpc_session>
	 * @return The return type is same as the T.
	 */
	template<class T>
	[[maybe_unused]] inline T get_current_caller() noexcept
	{
		if /**/ constexpr (detail::is_template_instance_of_v<std::shared_ptr, T>)
		{
			return detail::get_current_object<T>().lock();
		}
		else if constexpr (std::is_reference_v<T>)
		{
			return *detail::get_current_object<std::add_pointer_t<typename detail::remove_cvref_t<T>>>();
		}
		else
		{
			return detail::get_current_object<T>();
		}
	}
}

namespace asio2::detail
{
	struct wait_timer_op : public asio::coroutine
	{
		asio::steady_timer& timer_;

		wait_timer_op(asio::steady_timer& timer) : timer_(timer)
		{
		}

		template <typename Self>
		void operator()(Self& self, error_code ec = {})
		{
			detail::ignore_unused(ec);

			ASIO_CORO_REENTER(*this)
			{
				ASIO_CORO_YIELD
					timer_.async_wait(std::move(self));

				self.complete(get_last_error());
			}
		}
	};

	class data_filter_before_helper
	{
	public:
		template<class, class = void>
		struct has_member_data_filter_before_recv : std::false_type {};

		template<class T>
		struct has_member_data_filter_before_recv<T, std::void_t<decltype(
			std::declval<std::decay_t<T>&>().data_filter_before_recv(std::string_view{}))>> : std::true_type {};

		template<class, class = void>
		struct has_member_data_filter_before_send : std::false_type {};

		template<class T>
		struct has_member_data_filter_before_send<T, std::void_t<decltype(
			std::declval<std::decay_t<T>&>().data_filter_before_send(std::string_view{}))>> : std::true_type {};

		template<class derived_t>
		inline static std::string_view call_data_filter_before_recv(derived_t& derive, std::string_view data) noexcept
		{
			if constexpr (has_member_data_filter_before_recv<derived_t>::value)
				return derive.data_filter_before_recv(data);
			else
				return data;
		}

		template<class derived_t, class T>
		inline static auto call_data_filter_before_send(derived_t& derive, T&& data) noexcept
		{
			if constexpr (has_member_data_filter_before_send<derived_t>::value)
				return derive.data_filter_before_send(std::forward<T>(data));
			else
				return std::forward<T>(data);
		}
	};

	template<class derived_t>
	inline std::string_view call_data_filter_before_recv(derived_t& derive, std::string_view data) noexcept
	{
		return data_filter_before_helper::call_data_filter_before_recv(derive, data);
	}

	template<class derived_t, class T>
	inline auto call_data_filter_before_send(derived_t& derive, T&& data) noexcept
	{
		return data_filter_before_helper::call_data_filter_before_send(derive, std::forward<T>(data));
	}

	template<class, class = void>
	struct has_member_insert : std::false_type {};

	template<class T>
	struct has_member_insert<T, std::void_t<decltype(
		std::declval<std::decay_t<T>&>().insert(std::declval<std::decay_t<T>&>().begin(),
			std::string_view{}.begin(), std::string_view{}.end()))>> : std::true_type {};
}

namespace asio2
{
	// helper type for the visitor #4
	template<class... Ts>
	struct variant_overloaded : Ts... { using Ts::operator()...; };

	// explicit deduction guide (not needed as of C++20)
	template<class... Ts>
	variant_overloaded(Ts...) -> variant_overloaded<Ts...>;
}

#endif // !__ASIO2_UTIL_HPP__
