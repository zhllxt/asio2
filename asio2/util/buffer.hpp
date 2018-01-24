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
#include <cstring>
#include <memory>

// 
// <------------------------------------------------- m_capacity --------------------------------------------------->
// <.. m_data ....
//                    <= m_rpos 
//                                                             <= m_wpos
// 
// |----------------------------------------------------------------------------------------------------------------|
// |                  |                                        |                                                    |
// |----------------------------------------------------------------------------------------------------------------|
// 
//                    <---------------- size() ---------------->
//                    <= read_pos()                            <---------------------- remain() -------------------->
//                    <.. read_begin()....
//                    <.. data()....
//                                                             <= write_pos()
//                                                             <.. write_begin()....
// <------------------------------------------------- capacity() --------------------------------------------------->
// 

namespace asio2
{

	/**
	 * template buffer interface, the sizeof(T) must be equal to 1,otherwise the member function will be not correct.
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
			if (m_capacity == 0)
				m_capacity = 64;
			m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
		}

		explicit buffer(const T * data, std::size_t size) : m_capacity(size)
		{
			if (m_capacity == 0)
				m_capacity = 64;
			m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
			if (data && size > 0)
			{
				std::memcpy((void *)write_begin(), (const void *)data, size);
				write_bytes(size);
			}
		}

		explicit buffer(const std::size_t capacity, const T * data, std::size_t size) : m_capacity(capacity)
		{
			if (m_capacity == 0)
				m_capacity = 64;
			if (m_capacity < size)
				m_capacity = size;
			m_data = std::shared_ptr<T>(new T[m_capacity], std::default_delete<T[]>());
			if (data && size > 0)
			{
				std::memcpy((void *)write_begin(), (const void *)data, size);
				write_bytes(size);
			}
		}

		explicit buffer(std::size_t capacity, std::shared_ptr<T> buf, std::size_t size) : m_data(std::move(buf)), m_capacity(capacity), m_wpos(size)
		{
			assert(m_wpos <= m_capacity);
		}

		explicit buffer(std::size_t capacity, std::shared_ptr<T> buf, const T * data, std::size_t size) : m_data(std::move(buf)), m_capacity(capacity)
		{
			if (m_data && data && size > 0)
			{
				std::memcpy((void *)write_begin(), (const void *)data, size);
				write_bytes(size);
				assert(m_wpos <= m_capacity);
			}
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
			assert(m_rpos <= m_wpos);
			return (m_wpos - m_rpos);
		}

		/**
		 * @function : get the data length
		 */
		inline std::size_t length()
		{
			return size();
		}

		/**
		 * @function : get the buffer total capacity
		 */
		inline std::size_t capacity()
		{
			return (m_capacity);
		}

		/**
		 * @function : get the buffer remain length
		 */
		inline std::size_t remain()
		{
			assert(m_wpos <= m_capacity);
			return (m_capacity - m_wpos);
		}

		/**
		 * @function : reset the read position and write position to zero
		 */
		inline void reset()
		{
			m_rpos = 0;
			m_wpos = 0;
		}

		/**
		 * @function : get buffer begin pointer
		 */
		inline T * buffer_begin()
		{
			return (m_data.get());
		}

		/**
		 * @function : get data pointer( equal to read position pointer )
		 */
		inline T * data()
		{
			return read_begin();
		}

		/**
		 * @function : get read position pointer
		 */
		inline T * read_begin()
		{
			return (m_data.get() + m_rpos);
		}

		/**
		 * @function : get write position pointer
		 */
		inline T * write_begin()
		{
			return (m_data.get() + m_wpos);
		}

		/**
		 * @function : increase the read position
		 */
		inline void read_bytes(std::size_t bytes)
		{
			m_rpos += bytes;
			assert(m_rpos <= m_wpos);
		}

		/**
		 * @function : increase the write position
		 */
		inline void write_bytes(std::size_t bytes)
		{
			m_wpos += bytes;
			assert(m_wpos <= m_capacity);
		}

		/**
		 * @function : get read position
		 */
		inline std::size_t read_pos()
		{
			return m_rpos;
		}

		/**
		 * @function : get write position
		 */
		inline std::size_t write_pos()
		{
			return m_wpos;
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

		std::size_t m_capacity = 0;

		std::size_t m_rpos     = 0;

		std::size_t m_wpos     = 0;

	};

	using buffer_ptr = std::shared_ptr<buffer<uint8_t>>;

}

#endif // !__ASIO2_BUFFER_HPP__
