/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LANGUAGE_OBJC_H
#define BHO_PREDEF_LANGUAGE_OBJC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LANG_OBJC`

http://en.wikipedia.org/wiki/Objective-C[Objective-C] language.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__OBJC__+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_LANG_OBJC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__OBJC__)
#   undef BHO_LANG_OBJC
#   define BHO_LANG_OBJC BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_LANG_OBJC
#   define BHO_LANG_OBJC_AVAILABLE
#endif

#define BHO_LANG_OBJC_NAME "Objective-C"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_OBJC,BHO_LANG_OBJC_NAME)
