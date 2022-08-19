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

namespace asio2::detail
{
template <typename Signature>
struct function;

template <typename R, typename ... Args>
struct function<R(Args...)>
{
private:
	static constexpr std::size_t storage_size = sizeof(void*) * 3;
	static constexpr std::size_t storage_align = alignof(void*);

	template <typename T>
	struct uses_static_storage
		: std::bool_constant<true
			&& sizeof(T) <= storage_size
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
