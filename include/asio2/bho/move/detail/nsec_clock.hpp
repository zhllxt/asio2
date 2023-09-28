//  This code is based on Timer and Chrono code. Thanks to authors:
//
//  BHO.Timer:
//  Copyright Beman Dawes 1994-2007, 2011
//
//  BHO.Chrono:
//  Copyright Beman Dawes 2008
//  Copyright 2009-2010 Vicente J. Botet Escriba
//
//  Simplified and modified to be able to support exceptionless (-fno-exceptions).
//  BHO.Timer depends on BHO.Chorno wich uses bho::throw_exception.
//  And BHO.Chrono DLLs don't build in Win32 as there is no 
//  bho::throw_exception(std::exception const&) implementation
//  in BHO.Chrono:
//
//  Copyright 2020 Ion Gaztanaga
//
//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

//----------------------------------------------------------------------------//
//                                Windows                                     //
//----------------------------------------------------------------------------//
#ifndef BHO_MOVE_DETAIL_NSEC_CLOCK_HPP
#define BHO_MOVE_DETAIL_NSEC_CLOCK_HPP

#include <asio2/bho/config.hpp>
#include <asio2/bho/cstdint.hpp>
#include <cstdlib>


#   if (defined(_WIN32) || defined(__WIN32__) || defined(WIN32))
#     define BHO_MOVE_DETAIL_WINDOWS_API
#   elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
#     define BHO_MOVE_DETAIL_MAC_API
#   else
#     define BHO_MOVE_DETAIL_POSIX_API
#   endif

#if defined(BHO_MOVE_DETAIL_WINDOWS_API)

#include <asio2/bho/winapi/time.hpp>
#include <asio2/bho/winapi/timers.hpp>
#include <asio2/bho/winapi/get_last_error.hpp>
#include <asio2/bho/winapi/error_codes.hpp>
#include <asio2/bho/assert.hpp>
#include <asio2/bho/core/ignore_unused.hpp>

namespace bho { namespace move_detail {

template<int Dummy>
struct QPFHolder
{
   static inline double get_nsec_per_tic()
   {
      bho::winapi::LARGE_INTEGER_ freq;
      bho::winapi::BOOL_ r = bho::winapi::QueryPerformanceFrequency( &freq );
      bho::ignore_unused(r);
      BHO_ASSERT(r != 0 && "Boost::Move - get_nanosecs_per_tic Internal Error");

      return double(1000000000.0L / freq.QuadPart);
   }

   static const double nanosecs_per_tic;
};

template<int Dummy>
const double QPFHolder<Dummy>::nanosecs_per_tic = get_nsec_per_tic();

inline bho::uint64_t nsec_clock() BHO_NOEXCEPT
{
   double nanosecs_per_tic = QPFHolder<0>::nanosecs_per_tic;
   
   bho::winapi::LARGE_INTEGER_ pcount;
   unsigned times=0;
   while ( !bho::winapi::QueryPerformanceCounter( &pcount ) )
   {
      if ( ++times > 3 )
      {
         BHO_ASSERT("Boost::Move - QueryPerformanceCounter Internal Error");
         return 0u;
      }
   }

   return static_cast<bho::uint64_t>(nanosecs_per_tic * double(pcount.QuadPart));
}

}}  //namespace bho { namespace move_detail {

#elif defined(BHO_MOVE_DETAIL_MAC_API)

#include <mach/mach_time.h>  // mach_absolute_time, mach_timebase_info_data_t

inline bho::uint64_t nsec_clock() BHO_NOEXCEPT
{
   bho::uint64_t count = ::mach_absolute_time();

   mach_timebase_info_data_t info;
   mach_timebase_info(&info);
   return static_cast<bho::uint64_t>
      ( static_cast<double>(count)*(static_cast<double>(info.numer) / info.denom) );
}

#elif defined(BHO_MOVE_DETAIL_POSIX_API)

#include <time.h>

#  if defined(CLOCK_MONOTONIC_PRECISE)   //BSD
#     define BHO_MOVE_DETAIL_CLOCK_MONOTONIC CLOCK_MONOTONIC_PRECISE
#  elif defined(CLOCK_MONOTONIC_RAW)     //Linux
#     define BHO_MOVE_DETAIL_CLOCK_MONOTONIC CLOCK_MONOTONIC_RAW
#  elif defined(CLOCK_HIGHRES)           //Solaris
#     define BHO_MOVE_DETAIL_CLOCK_MONOTONIC CLOCK_HIGHRES
#  elif defined(CLOCK_MONOTONIC)         //POSIX (AIX, BSD, Linux, Solaris)
#     define BHO_MOVE_DETAIL_CLOCK_MONOTONIC CLOCK_MONOTONIC
#  else
#     error "No high resolution steady clock in your system, please provide a patch"
#  endif

inline bho::uint64_t nsec_clock() BHO_NOEXCEPT
{
   struct timespec count;
   ::clock_gettime(BHO_MOVE_DETAIL_CLOCK_MONOTONIC, &count);
   bho::uint64_t r = static_cast<bho::uint64_t>(count.tv_sec);
   r *= 1000000000U;
   r += static_cast<bho::uint64_t>(count.tv_nsec);
   return r;
}

#endif  // POSIX

namespace bho { namespace move_detail {

typedef bho::uint64_t nanosecond_type;

struct cpu_times
{
   nanosecond_type wall;
   nanosecond_type user;
   nanosecond_type system;

   void clear() { wall = user = system = 0; }
};


inline void get_cpu_times(bho::move_detail::cpu_times& current)
{
    current.wall = nsec_clock();
}


class cpu_timer
{
   public:

      //  constructor
      cpu_timer() BHO_NOEXCEPT                                   { start(); }

      //  observers
      bool          is_stopped() const BHO_NOEXCEPT              { return m_is_stopped; }
      cpu_times     elapsed() const BHO_NOEXCEPT;  // does not stop()

      //  actions
      void          start() BHO_NOEXCEPT;
      void          stop() BHO_NOEXCEPT;
      void          resume() BHO_NOEXCEPT; 

   private:
      cpu_times     m_times;
      bool          m_is_stopped;
};


//  cpu_timer  ---------------------------------------------------------------------//

inline void cpu_timer::start() BHO_NOEXCEPT
{
   m_is_stopped = false;
   get_cpu_times(m_times);
}

inline void cpu_timer::stop() BHO_NOEXCEPT
{
   if (is_stopped())
      return;
   m_is_stopped = true;
      
   cpu_times current;
   get_cpu_times(current);
   m_times.wall = (current.wall - m_times.wall);
   m_times.user = (current.user - m_times.user);
   m_times.system = (current.system - m_times.system);
}

inline cpu_times cpu_timer::elapsed() const BHO_NOEXCEPT
{
   if (is_stopped())
      return m_times;
   cpu_times current;
   get_cpu_times(current);
   current.wall -= m_times.wall;
   current.user -= m_times.user;
   current.system -= m_times.system;
   return current;
}

inline void cpu_timer::resume() BHO_NOEXCEPT
{
   if (is_stopped())
   {
      cpu_times current (m_times);
      start();
      m_times.wall   -= current.wall;
      m_times.user   -= current.user;
      m_times.system -= current.system;
   }
}



}  // namespace move_detail
}  // namespace bho

#endif   //BHO_MOVE_DETAIL_NSEC_CLOCK_HPP
