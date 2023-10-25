#ifndef BHO_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED
#define BHO_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

// bho/core/detail/sp_thread_yield.hpp
//
// inline void bost::core::sp_thread_yield();
//
//   Gives up the remainder of the time slice,
//   as if by calling sched_yield().
//
// Copyright 2008, 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/config.hpp>
#include <asio2/bho/config/pragma_message.hpp>

#if defined( _WIN32 ) || defined( __WIN32__ ) || defined( __CYGWIN__ )

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using SwitchToThread() in sp_thread_yield")
#endif

#include <asio2/bho/core/detail/sp_win32_sleep.hpp>

namespace bho
{
namespace core
{
namespace detail
{

inline void sp_thread_yield() BHO_NOEXCEPT
{
    SwitchToThread();
}

} // namespace detail

using bho::core::detail::sp_thread_yield;

} // namespace core
} // namespace bho

#elif defined(BHO_HAS_SCHED_YIELD)

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using sched_yield() in sp_thread_yield")
#endif

#ifndef _AIX
# include <sched.h>
#else
  // AIX's sched.h defines ::var which sometimes conflicts with Lambda's var
  extern "C" int sched_yield(void);
#endif

namespace bho
{
namespace core
{

inline void sp_thread_yield() BHO_NOEXCEPT
{
    sched_yield();
}

} // namespace core
} // namespace bho

#else

#if defined(BHO_SP_REPORT_IMPLEMENTATION)
  BHO_PRAGMA_MESSAGE("Using sp_thread_pause() in sp_thread_yield")
#endif

#include <asio2/bho/core/detail/sp_thread_pause.hpp>

namespace bho
{
namespace core
{

inline void sp_thread_yield() BHO_NOEXCEPT
{
    sp_thread_pause();
}

} // namespace core
} // namespace bho

#endif

#endif // #ifndef BHO_CORE_DETAIL_SP_THREAD_YIELD_HPP_INCLUDED
