/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_ALLOCATOR_HPP__
#define __ASIO2_ALLOCATOR_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <type_traits>
#include <utility>
#include <atomic>
#include <fstream>

namespace asio2::detail
{
	//static std::size_t max_size_ = 0;
	//inline void log_max_size(std::size_t size)
	//{
	//	if (size > max_size_)
	//	{
	//		max_size_ = size;

	//		std::fstream file("e:/maxsize/" + std::to_string(max_size_) + ".txt", std::ios::out | std::ios::binary);
	//		file << ' ';
	//	}
	//}

	/// see : boost\libs\asio\example\cpp11\allocation\server.cpp

	static constexpr std::size_t allocator_size = 1024;

	template<std::size_t N = allocator_size>
	struct size_op
	{
		static constexpr std::size_t size = N;
	};

	template<typename SizeN = size_op<allocator_size>, typename IsAtomicUse = std::false_type>
	class handler_memory;

	// Class to manage the memory to be used for handler-based custom allocation.
	// It contains a single block of memory which may be returned for allocation
	// requests. If the memory is in use when an allocation request is made, the
	// allocator delegates allocation to the global heap.
	template<typename SizeN>
	class handler_memory<SizeN, std::false_type>
	{
	public:
		explicit handler_memory() : in_use_(false) {}

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
			//log_max_size(size);
			if (!in_use_ && size < sizeof(storage_))
			{
				in_use_ = true;
				return &storage_;
			}
			else
			{
				return ::operator new(size);
			}
		}

		inline void deallocate(void* pointer)
		{
			if (pointer == &storage_)
			{
				in_use_ = false;
			}
			else
			{
				::operator delete(pointer);
			}
		}

	private:
		// Storage space used for handler-based custom memory allocation.
		typename std::aligned_storage<SizeN::size>::type storage_;

		// Whether the handler-based custom allocation storage has been used.
		bool in_use_;
	};

	template<typename SizeN>
	class handler_memory<SizeN, std::true_type>
	{
	public:
		handler_memory() { in_use_.clear(); }

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
			//log_max_size(size);
			if (!in_use_.test_and_set() && size < sizeof(storage_))
			{
				return &storage_;
			}
			else
			{
				return ::operator new(size);
			}
		}

		inline void deallocate(void* pointer)
		{
			if (pointer == &storage_)
			{
				in_use_.clear();
			}
			else
			{
				::operator delete(pointer);
			}
		}

	private:
		// Storage space used for handler-based custom memory allocation.
		typename std::aligned_storage<SizeN::size>::type storage_;

		// Whether the handler-based custom allocation storage has been used.
		std::atomic_flag in_use_;
	};

	// The allocator to be associated with the handler objects. This allocator only
	// needs to satisfy the C++11 minimal allocator requirements.
	template <typename T, typename N, typename B>
	class handler_allocator
	{
	public:
		using value_type = T;

		explicit handler_allocator(handler_memory<N, B>& mem)
			: memory_(mem)
		{
		}

		template <typename U, typename No, typename Bo>
		handler_allocator(const handler_allocator<U, No, Bo>& other) noexcept
			: memory_(other.memory_)
		{
		}

		inline bool operator==(const handler_allocator& other) const noexcept
		{
			return &memory_ == &other.memory_;
		}

		inline bool operator!=(const handler_allocator& other) const noexcept
		{
			return &memory_ != &other.memory_;
		}

		inline T* allocate(std::size_t n) const
		{
			return static_cast<T*>(memory_.allocate(sizeof(T) * n));
		}

		inline void deallocate(T* p, std::size_t /*n*/) const
		{
			return memory_.deallocate(p);
		}

	private:
		template <typename, typename, typename> friend class handler_allocator;

		// The underlying memory.
		handler_memory<N, B>& memory_;
	};

	// Wrapper class template for handler objects to allow handler memory
	// allocation to be customised. The allocator_type type and get_allocator()
	// member function are used by the asynchronous operations to obtain the
	// allocator. Calls to operator() are forwarded to the encapsulated handler.
	template <typename Handler, typename N, typename B>
	class custom_alloc_handler
	{
	public:
		using allocator_type = handler_allocator<Handler, N, B>;

		custom_alloc_handler(handler_memory<N, B>& m, Handler&& h)
			: memory_(m)
			, handler_(std::forward<Handler>(h))
		{
		}

		inline allocator_type get_allocator() const noexcept
		{
			return allocator_type(memory_);
		}

		template <typename ...Args>
		inline void operator()(Args&&... args)
		{
			handler_(std::forward<Args>(args)...);
		}

	private:
		handler_memory<N, B>& memory_;
		Handler handler_;
	};

	// Helper function to wrap a handler object to add custom allocation.
	// Must not be used in multithreading,Otherwise, it will cause program crash.
	template <typename Handler, typename N, typename B>
	inline custom_alloc_handler<Handler, N, B> make_allocator(handler_memory<N, B>& m, Handler&& h)
	{
		return custom_alloc_handler<Handler, N, B>(m, std::forward<Handler>(h));
	}

}

#endif // !__ASIO2_ALLOCATOR_HPP__
