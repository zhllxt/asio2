/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_BUFFER_WRAP_HPP__
#define __ASIO2_BUFFER_WRAP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstddef>
#include <limits>
#include <memory>
#include <type_traits>

#include <asio2/external/asio.hpp>

namespace asio2
{
	namespace detail
	{
		static std::size_t constexpr min_size = 512;

		template<class, class = std::void_t<>>
		struct buffer_has_limit : std::false_type {};

		template<class T>
		struct buffer_has_limit<T, std::void_t<decltype(T(std::size_t(0)))>> : std::true_type {};

		template<class, class = std::void_t<>>
		struct buffer_has_max_size : std::false_type {};

		template<class T>
		struct buffer_has_max_size<T, std::void_t<decltype(std::declval<T>().max_size())>> : std::true_type {};

		//template<typename T>
		//struct buffer_has_limit
		//{
		//private:
		//	template<typename U>
		//	static auto check(bool) -> decltype(U(std::size_t(0)), std::true_type());
		//	template<typename U>
		//	static std::false_type check(...);
		//public:
		//	static constexpr bool value = std::is_same_v<decltype(check<T>(true)), std::true_type>;
		//};

		//template<typename T>
		//struct buffer_has_max_size
		//{
		//private:
		//	template<typename U>
		//	static auto check(bool) -> decltype(std::declval<U>().max_size(), std::true_type());
		//	template<typename U>
		//	static std::false_type check(...);
		//public:
		//	static constexpr bool value = std::is_same_v<decltype(check<T>(true)), std::true_type>;
		//};

		// send callback indirect
		struct callback_helper
		{
			template<class F>
			typename std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(
				std::size_t(0)), std::true_type()), std::true_type>>
				static inline call(F& f, std::size_t bytes_sent)
			{
				f(bytes_sent);
			}

			template<class F>
			typename std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(),
				std::true_type()), std::true_type>>
				static inline call(F& f, std::size_t)
			{
				f();
			}
		};

		struct empty_buffer
		{
			using size_type = std::size_t;

			inline size_type size    ()    const noexcept { return 0; }
			inline size_type max_size()    const noexcept { return 0; }
			inline size_type capacity()    const noexcept { return 0; }
			inline auto      data    ()          noexcept { return asio::buffer(asio::const_buffer  ()); }
			inline auto      prepare (size_type) noexcept { return asio::buffer(asio::mutable_buffer()); }
			inline void      commit  (size_type) noexcept {}
			inline void      consume (size_type) noexcept {}
		};

		template<class B>
		struct proxy_buffer
		{
			using size_type = std::size_t;

			proxy_buffer() noexcept {}
			proxy_buffer(size_type) noexcept {}

			inline size_type size    ()      const noexcept { return b_->size    ( ); }
			inline size_type max_size()      const noexcept { return b_->max_size( ); }
			inline size_type capacity()      const noexcept { return b_->capacity( ); }
			inline auto      data    ()            noexcept { return b_->data    ( ); }
			inline auto      prepare (size_type n) noexcept { return b_->prepare (n); }
			inline void      commit  (size_type n) noexcept {        b_->commit  (n); }
			inline void      consume (size_type n) noexcept {        b_->consume (n); }

			inline void bind_buffer(B* b) { b_ = b; }

			B* b_ = nullptr;
		};
	}

	template<class buffer_t, bool has_limit = detail::buffer_has_limit<buffer_t>::value>
	class buffer_wrap;

	template<class buffer_t>
	class buffer_wrap<buffer_t, true> : public buffer_t
	{
	public:
		using buffer_type = buffer_t;
		using buffer_t::buffer_t;
		using size_type = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type max) : buffer_t(max) {}
		buffer_wrap(size_type pre, size_type max) : buffer_t(max), pre_(pre)
		{
			buffer_t::prepare(this->pre_);
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_t      & base()       noexcept { return (*this); }
		inline buffer_t const& base() const noexcept { return (*this); }

		inline size_type   pre_size() const noexcept { return this->pre_; }
		inline size_type   max_size() const noexcept
		{
			if constexpr (detail::buffer_has_max_size<buffer_t>::value)
				return buffer_t::max_size();
			else
				return (std::numeric_limits<size_type>::max)();
		}
		inline buffer_wrap& pre_size(size_type size) noexcept { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type     ) noexcept {                    return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}

	protected:
		size_type pre_ = detail::min_size;
	};

	template<class buffer_t>
	class buffer_wrap<buffer_t, false> : public buffer_t
	{
	public:
		using buffer_type = buffer_t;
		using buffer_t::buffer_t;
		using size_type = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type max) : buffer_t(), max_(max) {}
		buffer_wrap(size_type pre, size_type max) : buffer_t(), pre_(pre), max_(max)
		{
			buffer_t::prepare(this->pre_);
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_t      & base()       noexcept { return (*this); }
		inline buffer_t const& base() const noexcept { return (*this); }

		inline size_type   pre_size() const noexcept { return this->pre_; }
		inline size_type   max_size() const noexcept
		{
			if constexpr (detail::buffer_has_max_size<buffer_t>::value)
				return buffer_t::max_size();
			else
				return this->max_;
		}
		inline buffer_wrap& pre_size(size_type size) noexcept { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type size) noexcept { this->max_ = size; return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}

	protected:
		size_type pre_ = detail::min_size; // prepare size
		size_type max_ = (std::numeric_limits<size_type>::max)();
	};

	template<class B>
	class buffer_wrap<detail::proxy_buffer<B>, true> : public detail::proxy_buffer<B>
	{
	public:
		using proxy = detail::proxy_buffer<B>;
		using buffer_type = B;
		using detail::proxy_buffer<B>::proxy_buffer;
		using size_type = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type max) : proxy(max) {}
		buffer_wrap(size_type pre, size_type max) : proxy(max), pre_(pre)
		{
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_type      & base()       noexcept { return (*this->b_); }
		inline buffer_type const& base() const noexcept { return (*this->b_); }

		inline size_type   pre_size() const noexcept { return this->pre_; }
		inline size_type   max_size() const noexcept
		{
			if constexpr (detail::buffer_has_max_size<proxy>::value)
				return proxy::max_size();
			else
				return (std::numeric_limits<size_type>::max)();
		}
		inline buffer_wrap& pre_size(size_type size) noexcept { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type     ) noexcept {                    return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}

	protected:
		size_type pre_ = detail::min_size;
	};

	template<class B>
	class buffer_wrap<detail::proxy_buffer<B>, false> : public detail::proxy_buffer<B>
	{
	public:
		using proxy = detail::proxy_buffer<B>;
		using buffer_type = B;
		using detail::proxy_buffer<B>::proxy_buffer;
		using size_type = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type max) : proxy(), max_(max) {}
		buffer_wrap(size_type pre, size_type max) : proxy(), pre_(pre), max_(max)
		{
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_type      & base()       noexcept { return (*this->b_); }
		inline buffer_type const& base() const noexcept { return (*this->b_); }

		inline size_type   pre_size() const noexcept { return this->pre_; }
		inline size_type   max_size() const noexcept
		{
			if constexpr (detail::buffer_has_max_size<proxy>::value)
				return proxy::max_size();
			else
				return this->max_;
		}
		inline buffer_wrap& pre_size(size_type size) noexcept { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type size) noexcept { this->max_ = size; return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}

	protected:
		size_type pre_ = detail::min_size; // prepare size
		size_type max_ = (std::numeric_limits<size_type>::max)();
	};

	template<>
	class buffer_wrap<detail::empty_buffer, true> : public detail::empty_buffer
	{
	public:
		using buffer_type = detail::empty_buffer;
		using size_type   = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type           ) noexcept {}
		buffer_wrap(size_type, size_type) noexcept {}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline detail::empty_buffer      & base()       noexcept { return (*this); }
		inline detail::empty_buffer const& base() const noexcept { return (*this); }

		inline size_type      pre_size(         ) const noexcept { return 0; }
		inline size_type      max_size(         ) const noexcept { return 0; }
		inline buffer_wrap&   pre_size(size_type)       noexcept { return (*this); }
		inline buffer_wrap&   max_size(size_type)       noexcept { return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}
	};

	template<>
	class buffer_wrap<detail::empty_buffer, false> : public detail::empty_buffer
	{
	public:
		using buffer_type = detail::empty_buffer;
		using size_type   = std::size_t;

		 buffer_wrap() = default;
		~buffer_wrap() = default;

		buffer_wrap(size_type           ) noexcept {}
		buffer_wrap(size_type, size_type) noexcept {}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline detail::empty_buffer      & base()       noexcept { return (*this); }
		inline detail::empty_buffer const& base() const noexcept { return (*this); }

		inline size_type      pre_size(         ) const noexcept { return 0; }
		inline size_type      max_size(         ) const noexcept { return 0; }
		inline buffer_wrap&   pre_size(size_type)       noexcept { return (*this); }
		inline buffer_wrap&   max_size(size_type)       noexcept { return (*this); }

		inline std::string_view data_view() noexcept
		{
			auto databuf = this->data();
			return std::string_view{ reinterpret_cast<
							std::string_view::const_pointer>(databuf.data()), databuf.size() };
		}
	};

}

#endif // !__ASIO2_BUFFER_WRAP_HPP__
