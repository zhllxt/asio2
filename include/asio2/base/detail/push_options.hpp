/*
 * Copyright (c) 2017-2023 zhllxt
 * 
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

// No header guard

// the tests trigger deprecation warnings when compiled with msvc in C++17 mode
// warning STL4009: std::allocator<void> is deprecated in C++17
#if defined(_MSVC_LANG) && _MSVC_LANG > 201402
#   ifndef _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#	    define _SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
#   endif
#   ifndef _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#	    define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING
#   endif
#   ifndef _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
#	    define _SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
#   endif
#   ifndef _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#	    define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#   endif
#endif

#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable:4100) // warning C4100: Unreferenced formal parameters
#  pragma warning(disable:4189) // warning C4189: Local variables are initialized but not referenced
#  pragma warning(disable:4191) // asio inner : from FARPROC to cancel_io_ex_t is unsafe
#  pragma warning(disable:4267) // warning C4267: Convert from size_t to uint32_t may be loss data
#  pragma warning(disable:4311)
#  pragma warning(disable:4312)
#  pragma warning(disable:4505) // Unreferenced local function removed
#  pragma warning(disable:4702) // unreachable code
#  pragma warning(disable:4996) // warning STL4009: std::allocator<void> is deprecated in C++17
#  pragma warning(disable:5054) // Operator '|': deprecated between enumerations of different types
#endif

#if defined(__GNUC__) || defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Woverflow"
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wunused-function"
#  pragma GCC diagnostic ignored "-Wunused-local-typedefs"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#  pragma GCC diagnostic ignored "-Wpedantic"
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#  if((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) > 70101)
#    pragma GCC diagnostic ignored "-Wimplicit-fallthrough="
#    pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#  endif
#endif

#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Woverflow"
#  pragma clang diagnostic ignored "-Wunused-variable"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wunused-function"
#  pragma clang diagnostic ignored "-Wexceptions"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wunused-private-field"
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#  pragma clang diagnostic ignored "-Wpedantic"
#  pragma clang diagnostic ignored "-Wmissing-field-initializers"
#  pragma clang diagnostic ignored "-Wimplicit-fallthrough="
#  pragma clang diagnostic ignored "-Wunused-but-set-parameter"
#  pragma clang diagnostic ignored "-Wmicrosoft-template"
#endif


/*
 * see : https://github.com/retf/Boost.Application/pull/40
 */
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || \
	defined(_WINDOWS_) || defined(__WINDOWS__) || defined(__TOS_WIN__)
#	ifndef _WIN32_WINNT
#		if __has_include(<winsdkver.h>)
#			include <winsdkver.h>
#			define _WIN32_WINNT _WIN32_WINNT_WIN7
#		endif
#		if __has_include(<SDKDDKVer.h>)
#			include <SDKDDKVer.h>
#		endif
#	endif
#	if __has_include(<crtdbg.h>)
#		include <crtdbg.h>
#	endif
#endif
