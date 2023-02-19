/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_SHARED_MTX_HPP__
#define __ASIO2_SHARED_MTX_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <mutex>
#include <shared_mutex>

// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
// https://stackoverflow.com/questions/33608378/clang-thread-safety-annotation-and-shared-capabilities

// when compiled with "Visual Studio 2017 - Windows XP (v141_xp)"
// there is hasn't shared_mutex
#ifndef ASIO2_HAS_SHARED_MUTEX
	#if defined(_MSC_VER)
		#if defined(_HAS_SHARED_MUTEX)
			#if _HAS_SHARED_MUTEX
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
			#else
				#define ASIO2_HAS_SHARED_MUTEX 0
				#define asio2_shared_mutex std::mutex
				#define asio2_shared_lock  std::lock_guard
				#define asio2_unique_lock  std::lock_guard
			#endif
		#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
		#endif
	#else
				#define ASIO2_HAS_SHARED_MUTEX 1
				#define asio2_shared_mutex std::shared_mutex
				#define asio2_shared_lock  std::shared_lock
				#define asio2_unique_lock  std::unique_lock
	#endif
#endif


// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define ASIO2_CAPABILITY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define ASIO2_SCOPED_CAPABILITY \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define ASIO2_GUARDED_BY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define ASIO2_PT_GUARDED_BY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ASIO2_ACQUIRED_BEFORE(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ASIO2_ACQUIRED_AFTER(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define ASIO2_REQUIRES(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define ASIO2_REQUIRES_SHARED(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ASIO2_ACQUIRE(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ASIO2_ACQUIRE_SHARED(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define ASIO2_RELEASE(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define ASIO2_RELEASE_SHARED(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define ASIO2_RELEASE_GENERIC(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

#define ASIO2_TRY_ACQUIRE(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define ASIO2_TRY_ACQUIRE_SHARED(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define ASIO2_EXCLUDES(...) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASIO2_ASSERT_CAPABILITY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASIO2_ASSERT_SHARED_CAPABILITY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define ASIO2_RETURN_CAPABILITY(x) \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define ASIO2_NO_THREAD_SAFETY_ANALYSIS \
  ASIO2_THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

namespace asio2
{
	// Defines an annotated interface for mutexes.
	// These methods can be implemented to use any internal mutex implementation.
	class ASIO2_CAPABILITY("mutex") shared_mutexer
	{
	public:
		inline asio2_shared_mutex& native_handle() { return mutex_; }

		// Good software engineering practice dictates that mutexes should be private members,
		// because the locking mechanism used by a thread-safe class is part of its internal
		// implementation. However, private mutexes can sometimes leak into the public interface
		// of a class. Thread safety attributes follow normal C++ access restrictions, so if is
		// a private member of , then it is an error to write in an attribute.mucc.mu
	private:
		mutable asio2_shared_mutex mutex_;
	};

	// shared_locker is an RAII class that acquires a mutex in its constructor, and
	// releases it in its destructor.
	class ASIO2_SCOPED_CAPABILITY shared_locker
	{
	public:
		// Acquire mutex in shared mode, implicitly acquire *this and associate it with mutex.
		explicit shared_locker(shared_mutexer& m) ASIO2_ACQUIRE_SHARED(m) : lock_(m.native_handle()) {}

		// Release *this and all associated mutexes, if they are still held.
		// There is no warning if the scope was already unlocked before.
		// Note: can't use ASIO2_RELEASE_SHARED
		// @see: https://stackoverflow.com/questions/33608378/clang-thread-safety-annotation-and-shared-capabilities
		~shared_locker() ASIO2_RELEASE()
		{
		}

	private:
		asio2_shared_lock<asio2_shared_mutex> lock_;
	};

	// unique_locker is an RAII class that acquires a mutex in its constructor, and
	// releases it in its destructor.
	class ASIO2_SCOPED_CAPABILITY unique_locker
	{
	public:
		// Acquire mutex, implicitly acquire *this and associate it with mutex.
		explicit unique_locker(shared_mutexer& m) ASIO2_ACQUIRE(m) : lock_(m.native_handle()) {}

		// Release *this and all associated mutexes, if they are still held.
		// There is no warning if the scope was already unlocked before.
		~unique_locker() ASIO2_RELEASE()
		{
		}

	private:
		asio2_unique_lock<asio2_shared_mutex> lock_;
	};
}

#endif // !__ASIO2_SHARED_MTX_HPP__
