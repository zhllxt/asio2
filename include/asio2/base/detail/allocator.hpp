/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <mutex>

#include <asio2/config.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
	template<typename = void>
	inline void log_allocator_storage_size(bool is_lookfree, bool is_stack, std::size_t size)
	{
		static std::mutex mtx;
		static std::map<std::size_t, std::size_t> unlock_stack_map;
		static std::map<std::size_t, std::size_t> unlock_heaps_map;
		static std::map<std::size_t, std::size_t> atomic_stack_map;
		static std::map<std::size_t, std::size_t> atomic_heaps_map;

		std::lock_guard guard(mtx);

		if (is_lookfree)
		{
			if (is_stack)
				unlock_stack_map[size]++;
			else
				unlock_heaps_map[size]++;
		}
		else
		{
			if (is_stack)
				atomic_stack_map[size]++;
			else
				atomic_heaps_map[size]++;
		}

		static auto t1 = std::chrono::steady_clock::now();

		auto t2 = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() > 60)
		{
			t1 = std::chrono::steady_clock::now();

			std::string str;

			str += "\n";
			str += "allocator storage size test: ";

			if /**/ constexpr (sizeof(void*) == sizeof(std::uint64_t))
				str += "x64";
			else if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				str += "x86";
			else
				str += std::to_string(sizeof(void*)) + "bit";

			str += ", ";

		#if defined(_DEBUG) || defined(DEBUG)
			str += "Debug";
		#else
			str += "Release";
		#endif

		#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
			str += ", SSL";
		#endif

		#if ASIO2_OS_LINUX || ASIO2_OS_UNIX
			str += ", linux";
		#elif ASIO2_OS_WINDOWS
			str += ", windows";
		#elif ASIO2_OS_MACOS
			str += ", macos";
		#endif

			str += "\n";

			str += "------------------------------------------------------------\n";

			str += "unlock_stack_map\n";
			for (auto [len, num] : unlock_stack_map)
			{
				str += "  ";
				str += std::to_string(len); str += " : ";
				str += std::to_string(num); str += "\n";
			}

			str += "unlock_heaps_map\n";
			for (auto [len, num] : unlock_heaps_map)
			{
				str += "  ";
				str += std::to_string(len); str += " : ";
				str += std::to_string(num); str += "\n";
			}

			str += "atomic_stack_map\n";
			for (auto [len, num] : atomic_stack_map)
			{
				str += "  ";
				str += std::to_string(len); str += " : ";
				str += std::to_string(num); str += "\n";
			}

			str += "atomic_heaps_map\n";
			for (auto [len, num] : atomic_heaps_map)
			{
				str += "  ";
				str += std::to_string(len); str += " : ";
				str += std::to_string(num); str += "\n";
			}

			str += "------------------------------------------------------------\n";
			str += "\n";

			ASIO2_LOG_FATAL("{}", str);
		}
	}

	template<typename = void>
	inline void lockfree_allocator_threadsafe_test(std::size_t paddr, bool is_add)
	{
		static std::mutex mtx;
		static std::unordered_map<std::size_t, std::thread::id> addr_thread_map;

		std::lock_guard guard(mtx);

		if (is_add)
		{
			auto it = addr_thread_map.find(paddr);
			if (it == addr_thread_map.end())
			{
				addr_thread_map.emplace(paddr, std::this_thread::get_id());
			}
			else
			{
				if (it->second != std::this_thread::get_id())
				{
					ASIO2_ASSERT(false);
					throw 0;
				}
			}
		}
		else
		{
			addr_thread_map.erase(paddr);
		}
	}
#endif

	/// see : boost\libs\asio\example\cpp11\allocation\server.cpp

	template<typename IsLockFree>
	inline constexpr std::size_t calc_allocator_storage_size() noexcept
	{
#if   defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
		// if the user defined a custom allocator storage size, use it.
		return std::size_t(ASIO2_ALLOCATOR_STORAGE_SIZE);
#elif defined(_DEBUG) || defined(DEBUG)
		// debug mode just provide a simple size
		if constexpr (IsLockFree::value)
		{
			return std::size_t(1024);
		}
		else
		{
			return std::size_t(2048);
		}
#else
	#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
		if constexpr (IsLockFree::value)
		{
			if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				return std::size_t(332);
			else
				return std::size_t(664);
		}
		else
		{
			if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				return std::size_t(520);
			else
				return std::size_t(1024);
		}
	#else
		if constexpr (IsLockFree::value)
		{
			if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				return std::size_t(332);
			else
				return std::size_t(664);
		}
		else
		{
			if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				return std::size_t(512);
			else
				return std::size_t(1024);
		}
	#endif
#endif
	}

	template<std::size_t N>
	struct allocator_size_op
	{
		static constexpr std::size_t size = N;
	};

	struct allocator_fixed_size_tag {};

	template<std::size_t N>
	struct allocator_fixed_size_op : public allocator_fixed_size_tag
	{
		static constexpr std::size_t size = N;
	};

	template<class args_t>
	struct allocator_size_traits
	{
		template<class, class = void>
		struct has_member_size : std::false_type {};

		template<class T>
		struct has_member_size<T, std::void_t<decltype(T::allocator_storage_size)>> : std::true_type {};

		static constexpr std::size_t calc()
		{
			if constexpr (has_member_size<args_t>::value)
			{
				return args_t::allocator_storage_size;
			}
			else
			{
				return 0;
			}
		}

		static constexpr std::size_t value = calc();
	};

	// allocator storage sizer
	template<class args_t>
	using assizer = allocator_size_op<allocator_size_traits<args_t>::value>;

	template<typename IsLockFree, typename SizeN>
	inline constexpr std::size_t get_allocator_storage_size() noexcept
	{
		if constexpr (std::is_base_of_v<allocator_fixed_size_tag, detail::remove_cvref_t<SizeN>>)
		{
			return SizeN::size;
		}
		else
		{
		#if defined(ASIO2_ALLOCATOR_STORAGE_SIZE)
			// if the user defined a custom allocator storage size, use it.
			return std::size_t(ASIO2_ALLOCATOR_STORAGE_SIZE);
		#else
			if constexpr (SizeN::size >= std::size_t(64) && SizeN::size <= std::size_t(1024 * 1024))
			{
				return SizeN::size;
			}
			else
			{
				return calc_allocator_storage_size<IsLockFree>();
			}
		#endif
		}
	}

	/**
	 * @brief Class to manage the memory to be used for handler-based custom allocation.
	 * It contains a single block of memory which may be returned for allocation
	 * requests. If the memory is in use when an allocation request is made, the
	 * allocator delegates allocation to the global heap.
	 * @tparam IsLockFree - is lock free or not.
	 * @tparam SizeN - the single block of memory size.
	 */
	template<
		typename IsLockFree = std::true_type,
		typename SizeN = allocator_size_op<calc_allocator_storage_size<IsLockFree>()>>
	class handler_memory;

	/**
	 * @brief Class to manage the memory to be used for handler-based custom allocation.
	 * It contains a single block of memory which may be returned for allocation
	 * requests. If the memory is in use when an allocation request is made, the
	 * allocator delegates allocation to the global heap.
	 * @tparam IsLockFree - is lock free or not.
	 * @tparam SizeN - the single block of memory size.
	 */
	template<typename SizeN>
	class handler_memory<std::true_type, SizeN>
	{
	public:
		static constexpr std::size_t storage_size = get_allocator_storage_size<std::true_type, SizeN>();

		explicit handler_memory() noexcept : in_use_(false) {}

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
		#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
			lockfree_allocator_threadsafe_test(std::size_t(this), true);
		#endif

			if (!in_use_ && size < sizeof(storage_))
			{
			#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
				log_allocator_storage_size(true, true, size);
			#endif

				in_use_ = true;
				return &storage_;
			}
			else
			{
			#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
				log_allocator_storage_size(true, false, size);
			#endif

				return ::operator new(size);
			}
		}

		inline void deallocate(void* pointer) noexcept
		{
			// must erase when deallocate, otherwise if call server.stop -> server.start
			// then the test map will incorrect.
		#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
			lockfree_allocator_threadsafe_test(std::size_t(this), false);
		#endif

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
		typename std::aligned_storage<storage_size + sizeof(void*)>::type storage_{};

		// Whether the handler-based custom allocation storage has been used.
		bool in_use_{};
	};

	/**
	 * @brief Class to manage the memory to be used for handler-based custom allocation.
	 * It contains a single block of memory which may be returned for allocation
	 * requests. If the memory is in use when an allocation request is made, the
	 * allocator delegates allocation to the global heap.
	 * @tparam IsLockFree - is lock free or not.
	 * @tparam SizeN - the single block of memory size.
	 */
	template<typename SizeN>
	class handler_memory<std::false_type, SizeN>
	{
	public:
		static constexpr std::size_t storage_size = get_allocator_storage_size<std::false_type, SizeN>();

		handler_memory() noexcept { in_use_.clear(); }

		handler_memory(const handler_memory&) = delete;
		handler_memory& operator=(const handler_memory&) = delete;

		inline void* allocate(std::size_t size)
		{
			if (size < sizeof(storage_) && (!in_use_.test_and_set()))
			{
			#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
				log_allocator_storage_size(false, true, size);
			#endif

				return &storage_;
			}
			else
			{
			#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
				log_allocator_storage_size(false, false, size);
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
		typename std::aligned_storage<storage_size + sizeof(void*)>::type storage_{};

		// Whether the handler-based custom allocation storage has been used.
		std::atomic_flag in_use_{};
	};

#if defined(ASIO2_ENABLE_LOG)
	static_assert(handler_memory<std::true_type >::storage_size == calc_allocator_storage_size<std::true_type >());
	static_assert(handler_memory<std::false_type>::storage_size == calc_allocator_storage_size<std::false_type>());
#endif

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
