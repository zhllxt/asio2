// 
// https://github.com/lisyarus/movable_function
// 
// bench (Intel(R) Core(TM) i5-4590 CPU @ 3.30GHz, RAM 8.00 GB)
// 
// Assign light lambda (std::function)                took 2.03859s
// Assign light lambda (function)                     took 0.201078s
// Call light lambda (std::function)                  took 0.206337s
// Call light lambda (function)                       took 0.18224s
// counter = 200000000
// Assign heavy lambda (std::function)                took 15.1772s
// Assign heavy lambda (function)                     took 13.9959s
// Call heavy lambda (std::function)                  took 0.200089s
// Call heavy lambda (function)                       took 0.17089s
// counter = 200000000
// Assign function pointer (std::function)            took 1.83542s
// Assign function pointer (function)                 took 0.200447s
// Call function pointer (std::function)              took 0.199261s
// Call function pointer (function)                   took 0.226355s
// global_counter = 200000000
// Assign pointer to member (std::function)           took 1.88201s
// Assign pointer to member (function)                took 0.225137s
// Call pointer to member (std::function)             took 0.196708s
// Call pointer to member (function)                  took 0.225984s
// x.counter = 200000000
// 

#ifndef __ASIO2_FUNCTION_HPP__
#define __ASIO2_FUNCTION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <type_traits>
#include <memory>
#include <functional>

#include <asio2/config.hpp>
#include <asio2/base/log.hpp>

namespace asio2::detail
{
#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
	template<typename = void>
	inline void log_function_storage_size(bool is_stack, std::size_t size)
	{
		static std::mutex mtx;
		static std::map<std::size_t, std::size_t> stack_map;
		static std::map<std::size_t, std::size_t> heaps_map;

		std::lock_guard guard(mtx);

		if (is_stack)
			stack_map[size]++;
		else
			heaps_map[size]++;

		static auto t1 = std::chrono::steady_clock::now();

		auto t2 = std::chrono::steady_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() > 60)
		{
			t1 = std::chrono::steady_clock::now();

			std::string str;

			str += "\n";
			str += "function storage size test: ";

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

			str += "stack_map\n";
			for (auto [len, num] : stack_map)
			{
				str += "  ";
				str += std::to_string(len); str += " : ";
				str += std::to_string(num); str += "\n";
			}

			str += "heaps_map\n";
			for (auto [len, num] : heaps_map)
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
#endif
}

namespace asio2::detail
{
template<class args_t>
struct function_size_traits
{
	template<class, class = void>
	struct has_member_size : std::false_type {};

	template<class T>
	struct has_member_size<T, std::void_t<decltype(T::function_storage_size)>> : std::true_type {};

	static constexpr std::size_t calc()
	{
		if constexpr (has_member_size<args_t>::value)
		{
			return args_t::function_storage_size;
		}
		else
		{
			return 0;
		}
	}

	static constexpr std::size_t value = calc();
};

template <typename Signature, std::size_t StorageSize = 0>
struct function;

template <typename R, typename ... Args, std::size_t StorageSize>
struct function<R(Args...), StorageSize>
{
private:
	static constexpr std::size_t get_storage_size() noexcept
	{
	#ifdef ASIO2_FUNCTION_STORAGE_SIZE
		// if the user defined a custom function storage size, use it.
		return std::size_t(ASIO2_FUNCTION_STORAGE_SIZE);
	#else
		if constexpr (StorageSize >= sizeof(void*) && StorageSize <= std::size_t(1024))
		{
			return StorageSize;
		}
		else
		{
			if constexpr (sizeof(void*) == sizeof(std::uint32_t))
				return (sizeof(void*) * 10);
			else
				return (sizeof(void*) * 7);
		}
	#endif
	}

	static constexpr std::size_t storage_size  = get_storage_size();
	static constexpr std::size_t storage_align = alignof(void*);

	template <typename T>
	struct uses_static_storage
		: std::bool_constant<true
			&& sizeof (T) <= storage_size
			&& alignof(T) <= storage_align
			&& ((storage_align % alignof(T)) == 0)
			&& std::is_nothrow_move_constructible_v<T>
			>
	{};

public:

	using signature = R(Args...);

	function() noexcept = default;

	template <typename F>
	function(F && f)
	{
		assign(std::forward<F>(f));
	}

	function (function && other) noexcept
	{
		vtable_ = other.vtable_;
		if (vtable_)
			vtable_->move(std::addressof(other.storage_), std::addressof(storage_));
		other.reset();
	}
	function & operator = (function && other) noexcept
	{
		if (this == &other)
			return *this;

		reset();
		vtable_ = other.vtable_;
		if (vtable_)
			vtable_->move(std::addressof(other.storage_), std::addressof(storage_));
		other.reset();
		return *this;
	}

	function (function const &) = delete;
	function & operator = (function const &) = delete;

	~function()
	{
		reset();
	}

	// operator = (F && f) has strong exception guarantree:
	// if the assignment throws, the function remains unchanged

	template <typename F>
	std::enable_if_t<uses_static_storage<std::decay_t<F>>::value, function &>
		operator = (F && f)
	{
		reset();
		assign(std::forward<F>(f));
		return *this;
	}

	template <typename F>
	std::enable_if_t<!uses_static_storage<std::decay_t<F>>::value, function &>
		operator = (F && f)
	{
		function(std::forward<F>(f)).swap(*this);
		return *this;
	}

	explicit operator bool() const
	{
		return static_cast<bool>(vtable_);
	}

	template <typename ... Args1>
	R operator()(Args1 && ... args) const
	{
		if (!vtable_)
			throw std::bad_function_call();

		return vtable_->call(const_cast<void *>(static_cast<void const *>(&storage_)), std::forward<Args1>(args)...);
	}

	void reset()
	{
		if (!vtable_) return;

		vtable_->destroy(&storage_);
		vtable_ = nullptr;
	}

	void swap(function & other) noexcept
	{
		std::swap(*this, other);
	}

private:
	std::aligned_storage_t<storage_size, storage_align> storage_;

	struct vtable
	{
		using move_func = void(*)(void *, void *);
		using destroy_func = void(*)(void *);
		using call_func = R(*)(void *, Args&& ...);

		move_func move;
		destroy_func destroy;
		call_func call;
	};

	vtable * vtable_ = nullptr;

	template <typename F>
	void assign(F && f)
	{
		using T = std::decay_t<F>;

		if constexpr (uses_static_storage<T>::value)
		{
			new (reinterpret_cast<T *>(&storage_)) T(std::move(f));

		#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
			log_function_storage_size(true, sizeof(T));
		#endif

			static vtable m = {
				[](void * src, void * dst){ new (reinterpret_cast<T*>(dst)) T(std::move(*reinterpret_cast<T*>(src))); },
				[](void * src){ reinterpret_cast<T*>(src)->~T(); },
				[](void * src, Args&& ... args) -> R { return std::invoke(*reinterpret_cast<T*>(src), static_cast<Args&&>(args)...); }
			};

			vtable_ = &m;
		}
		else
		{
			*reinterpret_cast<T**>(&storage_) = new T(std::move(f));

		#if defined(ASIO2_ENABLE_LOG) && defined(ASIO2_ENABLE_LOG_STORAGE_SIZE)
			log_function_storage_size(false, sizeof(T));
		#endif

			static vtable m = {
				[](void * src, void * dst){ *reinterpret_cast<T**>(dst) = *reinterpret_cast<T**>(src); *reinterpret_cast<T**>(src) = nullptr; },
				[](void * src){ delete *reinterpret_cast<T**>(src); },
				[](void * src, Args&& ... args) -> R { return std::invoke(**reinterpret_cast<T**>(src), static_cast<Args&&>(args)...); }
			};

			vtable_ = &m;
		}
	}
};
}

#endif // !__ASIO2_FUNCTION_HPP__
