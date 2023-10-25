#ifndef BHO_CORE_DETAIL_SP_THREAD_SLEEP_HPP_INCLUDED
#define BHO_CORE_DETAIL_SP_THREAD_SLEEP_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// bho/core/detail/sp_thread_sleep.hpp
//
// inline void bost::core::sp_thread_sleep();
//
//   Cease execution for a while to yield to other threads,
//   as if by calling nanosleep() with an appropriate interval.
//
// Copyright 2008, 2020, 2023 Peter Dimov
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <asio2/bho/config/pragma_message.hpp>

#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using Sleep(1) in sp_thread_sleep")
#endif

#include <asio2/bho/core/detail/sp_win32_sleep.hpp>

namespace bho
{
namespace core
{
namespace detail
{

inline void sp_thread_sleep() BHO_NOEXCEPT
{
    Sleep( 1 );
}

} // namespace detail

using bho::core::detail::sp_thread_sleep;

} // namespace core
} // namespace bho

#elif defined(BHO_HAS_NANOSLEEP)

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using nanosleep() in sp_thread_sleep")
#endif

#include <time.h>

#if defined(BHO_HAS_PTHREADS) && !defined(__ANDROID__)
# include <pthread.h>
#endif

namespace bho
{
namespace core
{

inline void sp_thread_sleep() BHO_NOEXCEPT
{
#if defined(BHO_HAS_PTHREADS) && !defined(__ANDROID__)

    int oldst;
    pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, &oldst );

#endif

    // g++ -Wextra warns on {} or {0}
    struct timespec rqtp = { 0, 0 };

    // POSIX says that timespec has tv_sec and tv_nsec
    // But it doesn't guarantee order or placement

    rqtp.tv_sec = 0;
    rqtp.tv_nsec = 1000;

    nanosleep( &rqtp, 0 );

#if defined(BHO_HAS_PTHREADS) && !defined(__ANDROID__)

    pthread_setcancelstate( oldst, &oldst );

#endif

}

} // namespace core
} // namespace bho

#else

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using sp_thread_yield() in sp_thread_sleep")
#endif

#include <asio2/bho/core/detail/sp_thread_yield.hpp>

namespace bho
{
namespace core
{

inline void sp_thread_sleep() BHO_NOEXCEPT
{
    sp_thread_yield();
}

} // namespace core
} // namespace bho

#endif

#endif // #ifndef BHO_CORE_DETAIL_SP_THREAD_SLEEP_HPP_INCLUDED
