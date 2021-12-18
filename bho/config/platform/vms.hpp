//  (C) Copyright Artyom Beilis 2010.  
//  Use, modification and distribution are subject to the  
//  Boost Software License, Version 1.0. (See accompanying file  
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt) 

#ifndef BHO_CONFIG_PLATFORM_VMS_HPP 
#define BHO_CONFIG_PLATFORM_VMS_HPP 

#define BHO_PLATFORM "OpenVMS" 

#undef  BHO_HAS_STDINT_H 
#define BHO_HAS_UNISTD_H 
#define BHO_HAS_NL_TYPES_H 
#define BHO_HAS_GETTIMEOFDAY 
#define BHO_HAS_DIRENT_H 
#define BHO_HAS_PTHREADS 
#define BHO_HAS_NANOSLEEP 
#define BHO_HAS_CLOCK_GETTIME 
#define BHO_HAS_PTHREAD_MUTEXATTR_SETTYPE 
#define BHO_HAS_LOG1P 
#define BHO_HAS_EXPM1 
#define BHO_HAS_THREADS 
#undef  BHO_HAS_SCHED_YIELD 

#endif 
