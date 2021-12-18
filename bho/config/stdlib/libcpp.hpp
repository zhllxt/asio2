//  (C) Copyright Christopher Jefferson 2011.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  config for libc++
//  Might need more in here later.

#if !defined(_LIBCPP_VERSION)
#  include <ciso646>
#  if !defined(_LIBCPP_VERSION)
#      error "This is not libc++!"
#  endif
#endif

#define BHO_STDLIB "libc++ version " BHO_STRINGIZE(_LIBCPP_VERSION)

#define BHO_HAS_THREADS

#ifdef _LIBCPP_HAS_NO_VARIADICS
#    define BHO_NO_CXX11_HDR_TUPLE
#endif

// BHO_NO_CXX11_ALLOCATOR should imply no support for the C++11
// allocator model. The C++11 allocator model requires a conforming
// std::allocator_traits which is only possible with C++11 template
// aliases since members rebind_alloc and rebind_traits require it.
#if defined(_LIBCPP_HAS_NO_TEMPLATE_ALIASES)
#    define BHO_NO_CXX11_ALLOCATOR
#    define BHO_NO_CXX11_POINTER_TRAITS
#endif

#if __cplusplus < 201103
//
// These two appear to be somewhat useable in C++03 mode, there may be others...
//
//#  define BHO_NO_CXX11_HDR_ARRAY
//#  define BHO_NO_CXX11_HDR_FORWARD_LIST

#  define BHO_NO_CXX11_HDR_CODECVT
#  define BHO_NO_CXX11_HDR_CONDITION_VARIABLE
#  define BHO_NO_CXX11_HDR_EXCEPTION
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#  define BHO_NO_CXX11_HDR_MUTEX
#  define BHO_NO_CXX11_HDR_RANDOM
#  define BHO_NO_CXX11_HDR_RATIO
#  define BHO_NO_CXX11_HDR_REGEX
#  define BHO_NO_CXX11_HDR_SYSTEM_ERROR
#  define BHO_NO_CXX11_HDR_THREAD
#  define BHO_NO_CXX11_HDR_TUPLE
#  define BHO_NO_CXX11_HDR_TYPEINDEX
#  define BHO_NO_CXX11_HDR_UNORDERED_MAP
#  define BHO_NO_CXX11_HDR_UNORDERED_SET
#  define BHO_NO_CXX11_NUMERIC_LIMITS
#  define BHO_NO_CXX11_ALLOCATOR
#  define BHO_NO_CXX11_POINTER_TRAITS
#  define BHO_NO_CXX11_SMART_PTR
#  define BHO_NO_CXX11_HDR_FUNCTIONAL
#  define BHO_NO_CXX11_STD_ALIGN
#  define BHO_NO_CXX11_ADDRESSOF
#  define BHO_NO_CXX11_HDR_ATOMIC
#  define BHO_NO_CXX11_ATOMIC_SMART_PTR
#  define BHO_NO_CXX11_HDR_CHRONO
#  define BHO_NO_CXX11_HDR_TYPE_TRAITS
#  define BHO_NO_CXX11_HDR_FUTURE
#elif _LIBCPP_VERSION < 3700
//
// These appear to be unusable/incomplete so far:
//
#  define BHO_NO_CXX11_HDR_ATOMIC
#  define BHO_NO_CXX11_ATOMIC_SMART_PTR
#  define BHO_NO_CXX11_HDR_CHRONO
#  define BHO_NO_CXX11_HDR_TYPE_TRAITS
#  define BHO_NO_CXX11_HDR_FUTURE
#endif


#if _LIBCPP_VERSION < 3700
// libc++ uses a non-standard messages_base
#define BHO_NO_STD_MESSAGES
#endif

// C++14 features
#if (_LIBCPP_VERSION < 3700) || (__cplusplus <= 201402L)
#  define BHO_NO_CXX14_STD_EXCHANGE
#endif

// C++17 features
#if (_LIBCPP_VERSION < 4000) || (__cplusplus <= 201402L)
#  define BHO_NO_CXX17_STD_APPLY
#  define BHO_NO_CXX17_HDR_OPTIONAL
#  define BHO_NO_CXX17_HDR_STRING_VIEW
#  define BHO_NO_CXX17_HDR_VARIANT
#endif
#if (_LIBCPP_VERSION > 4000) && (__cplusplus > 201402L) && !defined(_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR)
#  define BHO_NO_AUTO_PTR
#endif
#if (_LIBCPP_VERSION > 4000) && (__cplusplus > 201402L) && !defined(_LIBCPP_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE)
#  define BHO_NO_CXX98_RANDOM_SHUFFLE
#endif
#if (_LIBCPP_VERSION > 4000) && (__cplusplus > 201402L) && !defined(_LIBCPP_ENABLE_CXX17_REMOVED_BINDERS)
#  define BHO_NO_CXX98_BINDERS
#endif

#ifdef __has_include
#if __has_include(<version>)
#include <version>

#if !defined(__cpp_lib_execution) || (__cpp_lib_execution < 201603L)
#  define BHO_NO_CXX17_HDR_EXECUTION
#endif
#if !defined(__cpp_lib_invoke) || (__cpp_lib_invoke < 201411L)
#define BHO_NO_CXX17_STD_INVOKE
#endif

#if !defined(__cpp_lib_bit_cast) || (__cpp_lib_bit_cast < 201806L) || !defined(__cpp_lib_bitops) || (__cpp_lib_bitops < 201907L) || !defined(__cpp_lib_endian) || (__cpp_lib_endian < 201907L)
#  define BHO_NO_CXX20_HDR_BIT
#endif
#if !defined(__cpp_lib_three_way_comparison) || (__cpp_lib_three_way_comparison < 201907L)
#  define BHO_NO_CXX20_HDR_COMPARE
#endif
#if !defined(__cpp_lib_ranges) || (__cpp_lib_ranges < 201911L)
#  define BHO_NO_CXX20_HDR_RANGES
#endif
#if !defined(__cpp_lib_barrier) || (__cpp_lib_barrier < 201907L)
#  define BHO_NO_CXX20_HDR_BARRIER
#endif
#if !defined(__cpp_lib_format) || (__cpp_lib_format < 201907L)
#  define BHO_NO_CXX20_HDR_FORMAT
#endif
#if !defined(__cpp_lib_source_location) || (__cpp_lib_source_location < 201907L)
#  define BHO_NO_CXX20_HDR_SOURCE_LOCATION
#endif
#if !defined(__cpp_lib_latch) || (__cpp_lib_latch < 201907L)
#  define BHO_NO_CXX20_HDR_SOURCE_LATCH
#endif
#if !defined(__cpp_lib_span) || (__cpp_lib_span < 202002L)
#  define BHO_NO_CXX20_HDR_SOURCE_SPAN
#endif
#if !defined(__cpp_lib_math_constants) || (__cpp_lib_math_constants < 201907L)
#  define BHO_NO_CXX20_HDR_SOURCE_NUMBERS
#endif
#if !defined(__cpp_lib_jthread) || (__cpp_lib_jthread < 201911L)
#  define BHO_NO_CXX20_HDR_SOURCE_STOP_TOKEN
#endif
#if !defined(__cpp_lib_concepts) || (__cpp_lib_concepts < 202002L)
#  define BHO_NO_CXX20_HDR_SOURCE_STOP_CONCEPTS
#endif
#if !defined(__cpp_lib_syncbuf) || (__cpp_lib_syncbuf < 201803L)
#  define BHO_NO_CXX20_HDR_SYNCSTREAM
#endif
#if !defined(__cpp_lib_coroutine) || (__cpp_lib_coroutine < 201902L)
#  define BHO_NO_CXX20_HDR_COROUTINE
#endif
#if !defined(__cpp_lib_semaphore) || (__cpp_lib_semaphore < 201907L)
#  define BHO_NO_CXX20_HDR_SEMAPHORE
#endif
#if !defined(__cpp_lib_concepts) || (__cpp_lib_concepts < 202002L)
#  define BHO_NO_CXX20_HDR_CONCEPTS
#endif

#if(_LIBCPP_VERSION < 9000) && !defined(BHO_NO_CXX20_HDR_SPAN)
// as_writable_bytes is missing.
#  define BHO_NO_CXX20_HDR_SPAN
#endif

#else
#define BHO_NO_CXX17_STD_INVOKE      // Invoke support is incomplete (no invoke_result)
#define BHO_NO_CXX17_HDR_EXECUTION
#endif
#else
#define BHO_NO_CXX17_STD_INVOKE      // Invoke support is incomplete (no invoke_result)
#define BHO_NO_CXX17_HDR_EXECUTION
#endif

#if _LIBCPP_VERSION < 10000  // What's the correct version check here?
#define BHO_NO_CXX17_ITERATOR_TRAITS
#endif

#if (_LIBCPP_VERSION <= 1101) && !defined(BHO_NO_CXX11_THREAD_LOCAL)
// This is a bit of a sledgehammer, because really it's just libc++abi that has no
// support for thread_local, leading to linker errors such as
// "undefined reference to `__cxa_thread_atexit'".  It is fixed in the
// most recent releases of libc++abi though...
#  define BHO_NO_CXX11_THREAD_LOCAL
#endif

#if defined(__linux__) && (_LIBCPP_VERSION < 6000) && !defined(BHO_NO_CXX11_THREAD_LOCAL)
// After libc++-dev is installed on Trusty, clang++-libc++ almost works,
// except uses of `thread_local` fail with undefined reference to
// `__cxa_thread_atexit`.
//
// clang's libc++abi provides an implementation by deferring to the glibc
// implementation, which may or may not be available (it is not on Trusty).
// clang 4's libc++abi will provide an implementation if one is not in glibc
// though, so thread local support should work with clang 4 and above as long
// as libc++abi is linked in.
#  define BHO_NO_CXX11_THREAD_LOCAL
#endif

#if defined(__has_include)
#if !__has_include(<shared_mutex>)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#elif __cplusplus <= 201103
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif
#elif __cplusplus < 201402
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif

#if !defined(BHO_NO_CXX14_HDR_SHARED_MUTEX) && (_LIBCPP_VERSION < 5000)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif

//  --- end ---
