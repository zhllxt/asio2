/*
 * COPYRIGHT (C) 2017, zhllxt
 *
 * author   : zhllxt
 * qq       : 37792738
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_BUFFER_HPP__
#define __ASIO2_BUFFER_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#if !defined(NDEBUG) && !defined(DEBUG) && !defined(_DEBUG)
#	define NDEBUG
#endif

#include <cassert>
#include <memory>


namespace asio2
{

	/**
	 * buffer interface
	 */
	template<typename T>
	class buffer
	{
	public:

		/**
		 * @construct
		 */
		explicit buffer(const std::size_t capacity) : m_capacity(capacity)
		{
			if (m_capacity > 0)
			{
				m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
			}
		}

		explicit buffer(const T * data, std::size_t len) : m_size(len), m_capacity(len)
		{
			if (m_capacity > 0)
			{
				m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
				std::memcpy((void *)m_data.get(), (const void *)data, m_size);
			}
		}

		explicit buffer(const std::size_t capacity, const T * data, std::size_t len) : m_size(len), m_capacity(capacity)
		{
			if (m_capacity > 0 && m_capacity >= m_size)
			{
				m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
				std::memcpy((void *)m_data.get(), (const void *)data, m_size);
			}
		}

		explicit buffer(std::shared_ptr<T> data, std::size_t size, std::size_t capacity) : m_data(data), m_size(size), m_capacity(capacity)
		{
		}

		/**
		 * @destruct
		 */
		~buffer() noexcept
		{
		}

		/**
		 * @function : get the data length
		 */
		inline std::size_t size()
		{
			return (m_size - m_offset);
		}

		/**
		 * @function : get the data length
		 */
		inline std::size_t length()
		{
			return size();
		}

		/**
		 * @function : reset the data length
		 */
		inline void resize(std::size_t size)
		{
			m_size = size;
		}

		/**
		 * @function : reset the data capacity
		 */
		inline void recapacity(std::size_t capacity)
		{
			m_capacity = capacity;
		}

		/**
		 * @function : get the buffer capacity length
		 */
		inline std::size_t capacity()
		{
			return (m_capacity - m_offset);
		}

		/**
		 * @function : reset the buffer offset
		 */
		inline void reoffset(std::size_t offset)
		{
			m_offset = offset;
		}

		/**
		 * @function : get the buffer offset
		 */
		inline std::size_t offset()
		{
			return m_offset;
		}

		/**
		 * @function : get data pointer
		 */
		inline T * data()
		{
			return (m_data.get() + m_offset);
		}

		/**
		 * @function : 
		 */
		inline operator T*() const
		{
			return data();
		}

	private:
		/// no copy construct function
		buffer(const buffer&) = delete;

		/// no operator equal function
		buffer& operator=(const buffer&) = delete;

		/// no construct by moving right
		buffer(buffer&&) = delete;

		/// no operator equal function by moving right
		buffer& operator=(buffer&&) = delete;

	protected:

		std::shared_ptr<T> m_data;

		std::size_t m_size = 0;

		std::size_t m_offset = 0;

		std::size_t m_capacity = 0;

	};

	using buffer_ptr = std::shared_ptr<buffer<uint8_t>>;

}

#endif // !__ASIO2_BUFFER_HPP__
