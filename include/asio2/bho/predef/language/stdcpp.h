/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LANGUAGE_STDCPP_H
#define BHO_PREDEF_LANGUAGE_STDCPP_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LANG_STDCPP`

http://en.wikipedia.org/wiki/C%2B%2B[Standard {CPP}] language.
If available, the year of the standard is detected as YYYY.MM.1 from the Epoch date.
Because of the way the {CPP} standardization process works the
defined version year will not be the commonly known year of the standard.
Specifically the defined versions are:

.Detected Version Number vs. {CPP} Standard Year
[options="header"]
|===
| Detected Version Number | Standard Year | {CPP} Standard
| 27.11.1 | 1998 | ISO/IEC 14882:1998
| 41.3.1 | 2011 | ISO/IEC 14882:2011
| 44.2.1 | 2014 | ISO/IEC 14882:2014
| 47.3.1 | 2017 | ISO/IEC 14882:2017
|===

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__cplusplus+` | {predef_detection}

| `+__cplusplus+` | YYYY.MM.1
|===
*/ // end::reference[]

#define BHO_LANG_STDCPP BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__cplusplus)
#   undef BHO_LANG_STDCPP
#   if (__cplusplus > 100)
#       define BHO_LANG_STDCPP BHO_PREDEF_MAKE_YYYYMM(__cplusplus)
#   else
#       define BHO_LANG_STDCPP BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LANG_STDCPP
#   define BHO_LANG_STDCPP_AVAILABLE
#endif

#define BHO_LANG_STDCPP_NAME "Standard C++"

/* tag::reference[]
= `BHO_LANG_STDCPPCLI`

http://en.wikipedia.org/wiki/C%2B%2B/CLI[Standard {CPP}/CLI] language.
If available, the year of the standard is detected as YYYY.MM.1 from the Epoch date.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__cplusplus_cli+` | {predef_detection}

| `+__cplusplus_cli+` | YYYY.MM.1
|===
*/ // end::reference[]

#define BHO_LANG_STDCPPCLI BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__cplusplus_cli)
#   undef BHO_LANG_STDCPPCLI
#   if (__cplusplus_cli > 100)
#       define BHO_LANG_STDCPPCLI BHO_PREDEF_MAKE_YYYYMM(__cplusplus_cli)
#   else
#       define BHO_LANG_STDCPPCLI BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LANG_STDCPPCLI
#   define BHO_LANG_STDCPPCLI_AVAILABLE
#endif

#define BHO_LANG_STDCPPCLI_NAME "Standard C++/CLI"

/* tag::reference[]
= `BHO_LANG_STDECPP`

http://en.wikipedia.org/wiki/Embedded_C%2B%2B[Standard Embedded {CPP}] language.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__embedded_cplusplus+` | {predef_detection}
|===
*/ // end::reference[]

#define BHO_LANG_STDECPP BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__embedded_cplusplus)
#   undef BHO_LANG_STDECPP
#   define BHO_LANG_STDECPP BHO_VERSION_NUMBER_AVAILABLE
#endif

#if BHO_LANG_STDECPP
#   define BHO_LANG_STDECPP_AVAILABLE
#endif

#define BHO_LANG_STDECPP_NAME "Standard Embedded C++"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_STDCPP,BHO_LANG_STDCPP_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_STDCPPCLI,BHO_LANG_STDCPPCLI_NAME)

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_STDECPP,BHO_LANG_STDECPP_NAME)
