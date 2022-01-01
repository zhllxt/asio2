//  (C) Copyright John Maddock 2001 - 2003. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  cygwin specific config options:

#define BHO_PLATFORM "Cygwin"
#define BHO_HAS_DIRENT_H
#define BHO_HAS_LOG1P
#define BHO_HAS_EXPM1

//
// Threading API:
// See if we have POSIX threads, if we do use them, otherwise
// revert to native Win threads.
#define BHO_HAS_UNISTD_H
#include <unistd.h>
#if defined(_POSIX_THREADS) && (_POSIX_THREADS+0 >= 0) && !defined(BHO_HAS_WINTHREADS)
#  define BHO_HAS_PTHREADS
#  define BHO_HAS_SCHED_YIELD
#  define BHO_HAS_GETTIMEOFDAY
#  define BHO_HAS_PTHREAD_MUTEXATTR_SETTYPE
//#  define BHO_HAS_SIGACTION
#else
#  if !defined(BHO_HAS_WINTHREADS)
#     define BHO_HAS_WINTHREADS
#  endif
#  define BHO_HAS_FTIME
#endif

//
// find out if we have a stdint.h, there should be a better way to do this:
//
#include <sys/types.h>
#ifdef _STDINT_H
#define BHO_HAS_STDINT_H
#endif
#if __GNUC__ > 5 && !defined(BHO_HAS_STDINT_H)
#   define BHO_HAS_STDINT_H
#endif

#include <cygwin/version.h>
#if (CYGWIN_VERSION_API_MAJOR == 0 && CYGWIN_VERSION_API_MINOR < 231)
/// Cygwin has no fenv.h
#define BHO_NO_FENV_H
#endif

// Cygwin has it's own <pthread.h> which breaks <shared_mutex> unless the correct compiler flags are used:
#ifndef BHO_NO_CXX14_HDR_SHARED_MUTEX
#include <pthread.h>
#if !(__XSI_VISIBLE >= 500 || __POSIX_VISIBLE >= 200112)
#  define BHO_NO_CXX14_HDR_SHARED_MUTEX
#endif
#endif

// boilerplate code:
#include <asio2/bho/config/detail/posix_features.hpp>

//
// Cygwin lies about XSI conformance, there is no nl_types.h:
//
#ifdef BHO_HAS_NL_TYPES_H
#  undef BHO_HAS_NL_TYPES_H
#endif




