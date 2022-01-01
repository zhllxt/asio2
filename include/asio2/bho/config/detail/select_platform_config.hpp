//  Boost compiler configuration selection header file

//  (C) Copyright John Maddock 2001 - 2002. 
//  (C) Copyright Jens Maurer 2001. 
//  Use, modification and distribution are subject to the 
//  Boost Software License, Version 1.0. (See accompanying file 
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

// locate which platform we are on and define BHO_PLATFORM_CONFIG as needed.
// Note that we define the headers to include using "header_name" not
// <header_name> in order to prevent macro expansion within the header
// name (for example "linux" is a macro on linux systems).

#if (defined(linux) || defined(__linux) || defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)) && !defined(_CRAYC)
// linux, also other platforms (Hurd etc) that use GLIBC, should these really have their own config headers though?
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/linux.hpp"

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
// BSD:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/bsd.hpp"

#elif defined(sun) || defined(__sun)
// solaris:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/solaris.hpp"

#elif defined(__sgi)
// SGI Irix:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/irix.hpp"

#elif defined(__hpux)
// hp unix:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/hpux.hpp"

#elif defined(__CYGWIN__)
// cygwin is not win32:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/cygwin.hpp"

#elif defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
// win32:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/win32.hpp"

#elif defined(__HAIKU__)
// Haiku
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/haiku.hpp"

#elif defined(__BEOS__)
// BeOS
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/beos.hpp"

#elif defined(macintosh) || defined(__APPLE__) || defined(__APPLE_CC__)
// MacOS
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/macos.hpp"

#elif defined(__TOS_MVS__)
// IBM z/OS
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/zos.hpp"

#elif defined(__IBMCPP__) || defined(_AIX)
// IBM AIX
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/aix.hpp"

#elif defined(__amigaos__)
// AmigaOS
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/amigaos.hpp"

#elif defined(__QNXNTO__)
// QNX:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/qnxnto.hpp"

#elif defined(__VXWORKS__)
// vxWorks:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/vxworks.hpp"

#elif defined(__SYMBIAN32__) 
// Symbian: 
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/symbian.hpp" 

#elif defined(_CRAYC)
// Cray:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/cray.hpp" 

#elif defined(__VMS) 
// VMS:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/vms.hpp" 

#elif defined(__CloudABI__)
// Nuxi CloudABI:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/cloudabi.hpp"

#elif defined (__wasm__)
// Web assembly:
#  define BHO_PLATFORM_CONFIG "asio2/bho/config/platform/wasm.hpp"

#else

#  if defined(unix) \
      || defined(__unix) \
      || defined(_XOPEN_SOURCE) \
      || defined(_POSIX_SOURCE)

   // generic unix platform:

#  ifndef BHO_HAS_UNISTD_H
#     define BHO_HAS_UNISTD_H
#  endif

#  include <asio2/bho/config/detail/posix_features.hpp>

#  endif

#  if defined (BHO_ASSERT_CONFIG)
      // this must come last - generate an error if we don't
      // recognise the platform:
#     error "Unknown platform - please configure and report the results to boost.org"
#  endif

#endif

#if 0
//
// This section allows dependency scanners to find all the files we *might* include:
//
#  include "asio2/bho/config/platform/linux.hpp"
#  include "asio2/bho/config/platform/bsd.hpp"
#  include "asio2/bho/config/platform/solaris.hpp"
#  include "asio2/bho/config/platform/irix.hpp"
#  include "asio2/bho/config/platform/hpux.hpp"
#  include "asio2/bho/config/platform/cygwin.hpp"
#  include "asio2/bho/config/platform/win32.hpp"
#  include "asio2/bho/config/platform/beos.hpp"
#  include "asio2/bho/config/platform/macos.hpp"
#  include "asio2/bho/config/platform/zos.hpp"
#  include "asio2/bho/config/platform/aix.hpp"
#  include "asio2/bho/config/platform/amigaos.hpp"
#  include "asio2/bho/config/platform/qnxnto.hpp"
#  include "asio2/bho/config/platform/vxworks.hpp"
#  include "asio2/bho/config/platform/symbian.hpp" 
#  include "asio2/bho/config/platform/cray.hpp" 
#  include "asio2/bho/config/platform/vms.hpp" 
#  include <asio2/bho/config/detail/posix_features.hpp>



#endif

