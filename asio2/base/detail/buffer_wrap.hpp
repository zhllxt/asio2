/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

#include <asio2/base/selector.hpp>

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
			typename std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(std::size_t(0)), std::true_type()), std::true_type>>
				static inline call(F& f, std::size_t bytes_sent)
			{
				f(bytes_sent);
			}

			template<class F>
			typename std::enable_if_t<std::is_same_v<decltype(std::declval<F>()(), std::true_type()), std::true_type>>
				static inline call(F& f, std::size_t)
			{
				f();
			}
		};

		struct empty_buffer
		{
			using size_type = std::size_t;

			inline size_type size() const { return 0; }
			inline size_type max_size() const { return 0; }
			inline size_type capacity() const { return 0; }
			inline auto data() const { return asio::buffer(asio::const_buffer()); }
			inline auto prepare(size_type) { return asio::buffer(asio::mutable_buffer()); }
			inline void commit(size_type) {}
			inline void consume(size_type) {}
		};
	}

	template<class buffer_t, bool has_limit = detail::buffer_has_limit<buffer_t>::value> class buffer_wrap;

	template<class buffer_t>
	class buffer_wrap<buffer_t, true> : public buffer_t
	{
	public:
		using buffer_type = buffer_t;
		using buffer_t::buffer_t;
		using size_type = std::size_t;

		~buffer_wrap() = default;

		buffer_wrap() = default;

		buffer_wrap(size_type max) : buffer_t(max) {}
		buffer_wrap(size_type pre, size_type max) : buffer_t(max), pre_(pre)
		{
			buffer_t::prepare(this->pre_);
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_t & base() { return (*this); }
		inline buffer_t const& base() const { return (*this); }

		inline size_type pre_size() const { return this->pre_; }
		inline size_type max_size() const
		{
			if constexpr (detail::buffer_has_max_size<buffer_t>::value)
				return buffer_t::max_size();
			else
				return (std::numeric_limits<size_type>::max)();
		}
		inline buffer_wrap& pre_size(size_type size) { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type) { return (*this); }

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

		~buffer_wrap() = default;

		buffer_wrap() = default;

		buffer_wrap(size_type max) : buffer_t(), max_(max) {}
		buffer_wrap(size_type pre, size_type max) : buffer_t(), pre_(pre), max_(max)
		{
			buffer_t::prepare(this->pre_);
		}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline buffer_t & base() { return (*this); }
		inline buffer_t const& base() const { return (*this); }

		inline size_type pre_size() const { return this->pre_; }
		inline size_type max_size() const
		{
			if constexpr (detail::buffer_has_max_size<buffer_t>::value)
				return buffer_t::max_size();
			else
				return this->max_;
		}
		inline buffer_wrap& pre_size(size_type size) { this->pre_ = size; return (*this); }
		inline buffer_wrap& max_size(size_type size) { this->max_ = size; return (*this); }

	protected:
		size_type pre_ = detail::min_size; // prepare size
		size_type max_ = (std::numeric_limits<size_type>::max)();
	};

	template<>
	class buffer_wrap<detail::empty_buffer, true> : public detail::empty_buffer
	{
	public:
		using buffer_type = detail::empty_buffer;
		using size_type = std::size_t;

		~buffer_wrap() = default;

		buffer_wrap() = default;

		buffer_wrap(size_type) {}
		buffer_wrap(size_type, size_type) {}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline detail::empty_buffer & base() { return (*this); }
		inline detail::empty_buffer const& base() const { return (*this); }

		inline size_type pre_size() const { return 0; }
		inline size_type max_size() const { return 0; }
		inline buffer_wrap& pre_size(size_type) { return (*this); }
		inline buffer_wrap& max_size(size_type) { return (*this); }
	};

	template<>
	class buffer_wrap<detail::empty_buffer, false> : public detail::empty_buffer
	{
	public:
		using buffer_type = detail::empty_buffer;
		using size_type = std::size_t;

		~buffer_wrap() = default;

		buffer_wrap() = default;

		buffer_wrap(size_type) {}
		buffer_wrap(size_type, size_type) {}

		buffer_wrap(buffer_wrap&& other) = default;
		buffer_wrap(buffer_wrap const& other) = default;

		buffer_wrap& operator=(buffer_wrap&& other) = default;
		buffer_wrap& operator=(buffer_wrap const& other) = default;

		inline detail::empty_buffer & base() { return (*this); }
		inline detail::empty_buffer const& base() const { return (*this); }

		inline size_type pre_size() const { return 0; }
		inline size_type max_size() const { return 0; }
		inline buffer_wrap& pre_size(size_type) { return (*this); }
		inline buffer_wrap& max_size(size_type) { return (*this); }
	};

}

#endif // !__ASIO2_BUFFER_WRAP_HPP__
