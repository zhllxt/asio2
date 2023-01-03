/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_LINEAR_BUFFER_HPP__
#define __ASIO2_LINEAR_BUFFER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <limits>
#include <memory>
#include <vector>

#include <asio2/external/asio.hpp>

namespace asio2
{
	template<class Container>
	class basic_linear_buffer : protected Container
	{
	public:
		/// The type of allocator used.
		using allocator_type = typename Container::allocator_type;

		using size_type = typename Container::size_type;

		/// The type used to represent the input sequence as a list of buffers.
		using const_buffers_type = asio::const_buffer;

		/// The type used to represent the output sequence as a list of buffers.
		using mutable_buffers_type = asio::mutable_buffer;

		/// Destructor
		~basic_linear_buffer() = default;

		 basic_linear_buffer() = default;

		explicit basic_linear_buffer(size_type max) noexcept : Container(), max_(max) {}

		basic_linear_buffer(basic_linear_buffer&& other) = default;
		basic_linear_buffer(basic_linear_buffer const& other) = default;

		basic_linear_buffer& operator=(basic_linear_buffer&& other) = default;
		basic_linear_buffer& operator=(basic_linear_buffer const& other) = default;

		/// Returns the size of the input sequence.
		inline size_type size() const noexcept
		{
			return (wpos_ - rpos_);
		}

		/// Return the maximum sum of the input and output sequence sizes.
		inline size_type max_size() const noexcept
		{
			return max_;
		}

		/// Return the maximum sum of input and output sizes that can be held without an allocation.
		inline size_type capacity() const noexcept
		{
			return Container::capacity();
		}

		/// Get a list of buffers that represent the input sequence.
		inline const_buffers_type data() const noexcept
		{
			return { Container::data() + rpos_, wpos_ - rpos_ };
		}

		/** Get a list of buffers that represent the output sequence, with the given size.

			@throws std::length_error if `size() + n` exceeds `max_size()`.

			@note All previous buffers sequences obtained from
			calls to @ref data or @ref prepare are invalidated.
		*/
		inline mutable_buffers_type prepare(size_type n)
		{
			size_type const cap = Container::capacity();
			if (n <= cap - wpos_)
			{
				Container::resize(wpos_ + n);
				// existing capacity is sufficient
				return{ Container::data() + wpos_, n };
			}
			size_type const size = this->size();
			if (n <= cap - size)
			{
				// after a memmove,
				// existing capacity is sufficient
				if (size > 0)
					std::memmove(Container::data(), Container::data() + rpos_, size);
				rpos_ = 0;
				wpos_ = size;
				Container::resize(wpos_ + n);
				return { Container::data() + wpos_, n };
			}
			// enforce maximum capacity
			if (n > max_ - size)
				asio::detail::throw_exception(std::length_error{ "basic_linear_buffer overflow" });
			// allocate a new buffer
			size_type const new_size = (std::max<size_type>)((std::min<size_type>)(
				max_,
				(std::max<size_type>)(2 * cap, wpos_ + n)), min_size);
			Container::resize(new_size);
			Container::resize(wpos_ + n);
			return { Container::data() + wpos_, n };
		}

		/** Move bytes from the output sequence to the input sequence.

			@param n The number of bytes to move. If this is larger than
			the number of bytes in the output sequences, then the entire
			output sequences is moved.

			@note All previous buffers sequences obtained from
			calls to @ref data or @ref prepare are invalidated.
		*/
		inline void commit(size_type n) noexcept
		{
			wpos_ += (std::min<size_type>)(n, Container::size() - wpos_);
		}

		/** Remove bytes from the input sequence.

			If `n` is greater than the number of bytes in the input
			sequence, all bytes in the input sequence are removed.

			@note All previous buffers sequences obtained from
			calls to @ref data or @ref prepare are invalidated.
		*/
		inline void consume(size_type n) noexcept
		{
			if (n >= wpos_ - rpos_)
			{
				wpos_ = 0;
				rpos_ = 0;
				Container::resize(0);
				return;
			}
			rpos_ += n;
		}

		inline void shrink_to_fit()
		{
			Container::shrink_to_fit();
		}

	protected:
		size_type rpos_ = 0;
		size_type wpos_ = 0;
		size_type max_ = (std::numeric_limits<size_type>::max)();

		static size_type constexpr min_size = 512;
	};

	using linear_buffer = basic_linear_buffer<std::vector<char>>;
}

#endif // !__ASIO2_LINEAR_BUFFER_HPP__
