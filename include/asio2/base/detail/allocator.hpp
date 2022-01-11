/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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

#include <asio2/base/log.hpp>

namespace asio2::detail
{
	//template<typename = void>
	//inline void log_max_size(std::size_t size)
	//{
	//	static std::size_t max_size_ = 0;

	//	if (size > max_size_)
	//	{
	//		max_size_ = size;

	//		ASIO2_LOG(spdlog::level::critical, "max_size : {}", max_size_);
	//	}
	//}

	/// see : boost\libs\asio\example\cpp11\allocation\server.cpp

	// after test, in general situation:
	// 32 bit exe:
	// debug   mode : the max allocated size is 1304 and has very many times. 163 * 8 = 1304
	// release mode : the max allocated size is 456  and has very many times. 57  * 8 = 456
	// 64 bit exe:
	// debug   mode : the max allocated size is 2200 and has very many times. 275 * 8 = 2200
	// release mode : the max allocated size is 856  and has very many times. 107 * 8 = 856

#if defined(_DEBUG) || defined(DEBUG)
	template<typename = void>
	inline constexpr std::size_t allocator_size() noexcept
	{
		if constexpr (sizeof(void *) == sizeof(std::uint32_t))
			return std::size_t(1304 + 8);
		else
			return std::size_t(2200 + 8);
	}
#else
	template<typename = void>
	inline constexpr std::size_t allocator_size() noexcept
	{
		if constexpr (sizeof(void *) == sizeof(std::uint32_t))
			return std::size_t(456 + 64);
		else
			return std::size_t(856 + 64);
	}
#endif

	template<std::size_t N = allocator_size()>
	struct size_op
	{
		static constexpr std::size_t size = N;
	};

	template<typename SizeN = size_op<allocator_size()>, typename IsAtomicUse = std::false_type>
	class handler_memory;

#if defined(ASIO2_ENABLE_LOG) && (defined(_DEBUG) || defined(DEBUG))
	static std::size_t __unlock_use_counter__ = 0;
	static std::size_t __unlock_new_counter__ = 0;
	static std::size_t __atomic_use_counter__ = 0;
	static std::size_t __atomic_new_counter__ = 0;
#endif

	// Class to manage the memory to be used for handler-based custom allocation.
	// It contains a single block of memory which may be returned for allocation
	// requests. If the memory is in use when an allocation request is made, the
	// allocator delegates allocation to the global heap.
	template<typename SizeN>
	class handler_memory<SizeN, std::false_type>
	{
	public:
		explicit handler_memory() noexcept : in_use_(false) {}

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
			//log_max_size(size);

			if (!in_use_ && size < sizeof(storage_))
			{
			#if defined(ASIO2_ENABLE_LOG) && (defined(_DEBUG) || defined(DEBUG))
				__unlock_use_counter__++;
			#endif

				in_use_ = true;
				return &storage_;
			}
			else
			{
			#if defined(ASIO2_ENABLE_LOG) && (defined(_DEBUG) || defined(DEBUG))
				__unlock_new_counter__++;
			#endif

				return ::operator new(size);
			}
		}

		inline void deallocate(void* pointer) noexcept
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
		typename std::aligned_storage<SizeN::size>::type storage_{};

		// Whether the handler-based custom allocation storage has been used.
		bool in_use_{};
	};

	template<typename SizeN>
	class handler_memory<SizeN, std::true_type>
	{
	public:
		handler_memory() noexcept { in_use_.clear(); }

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
			//log_max_size(size);

			if (size < sizeof(storage_) && (!in_use_.test_and_set()))
			{
			#if defined(ASIO2_ENABLE_LOG) && (defined(_DEBUG) || defined(DEBUG))
				__atomic_use_counter__++;
			#endif

				return &storage_;
			}
			else
			{
			#if defined(ASIO2_ENABLE_LOG) && (defined(_DEBUG) || defined(DEBUG))
				__atomic_new_counter__++;
			#endif

				return ::operator new(size);
			}
		}

		inline void deallocate(void* pointer) noexcept
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
		typename std::aligned_storage<SizeN::size>::type storage_{};

		// Whether the handler-based custom allocation storage has been used.
		std::atomic_flag in_use_{};
	};

	// The allocator to be associated with the handler objects. This allocator only
	// needs to satisfy the C++11 minimal allocator requirements.
	template <typename T, typename N, typename B>
	class handler_allocator
	{
	public:
		using value_type = T;

		explicit handler_allocator(handler_memory<N, B>& mem) noexcept
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

		inline void deallocate(T* p, std::size_t /*n*/) const noexcept
		{
			return memory_.deallocate(p);
		}

	private:
		template <typename, typename, typename> friend class handler_allocator;

		// The underlying memory.
		handler_memory<N, B>& memory_{};
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

		custom_alloc_handler(handler_memory<N, B>& m, Handler&& h) noexcept
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
		handler_memory<N, B>& memory_{};
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
