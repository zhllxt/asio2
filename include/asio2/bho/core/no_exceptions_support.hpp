#ifndef BHO_CORE_NO_EXCEPTIONS_SUPPORT_HPP
#define BHO_CORE_NO_EXCEPTIONS_SUPPORT_HPP

#if defined(_MSC_VER)
#  pragma once
#endif

//----------------------------------------------------------------------
// (C) Copyright 2004 Pavel Vozenilek.
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// This file contains helper macros used when exception support may be
// disabled (as indicated by macro BHO_NO_EXCEPTIONS).
//
// Before picking up these macros you may consider using RAII techniques
// to deal with exceptions - their syntax can be always the same with 
// or without exception support enabled.
//----------------------------------------------------------------------

#include <asio2/bho/config.hpp>
#include <asio2/bho/config/workaround.hpp>

#if !(defined BHO_NO_EXCEPTIONS)
#    define BHO_TRY { try
#    define BHO_CATCH(x) catch(x)
#    define BHO_RETHROW throw;
#    define BHO_CATCH_END }
#else
#    if BHO_WORKAROUND(BHO_BORLANDC, BHO_TESTED_AT(0x564))
#        define BHO_TRY { if ("")
#        define BHO_CATCH(x) else if (!"")
#    elif !defined(BHO_MSVC) || BHO_MSVC >= 1900
#        define BHO_TRY { if (true)
#        define BHO_CATCH(x) else if (false)
#    else
         // warning C4127: conditional expression is constant
#        define BHO_TRY { \
             __pragma(warning(push)) \
             __pragma(warning(disable: 4127)) \
             if (true) \
             __pragma(warning(pop))
#        define BHO_CATCH(x) else \
             __pragma(warning(push)) \
             __pragma(warning(disable: 4127)) \
             if (false) \
             __pragma(warning(pop))
#    endif
#    define BHO_RETHROW
#    define BHO_CATCH_END }
#endif


#endif 
