/*
Copyright Rene Rivera 2011-2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LANGUAGE_STDC_H
#define BHO_PREDEF_LANGUAGE_STDC_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LANG_STDC`

http://en.wikipedia.org/wiki/C_(programming_language)[Standard C] language.
If available, the year of the standard is detected as YYYY.MM.1 from the Epoch date.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__STDC__+` | {predef_detection}

| `+__STDC_VERSION__+` | V.R.P
|===
*/ // end::reference[]

#define BHO_LANG_STDC BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__STDC__)
#   undef BHO_LANG_STDC
#   if defined(__STDC_VERSION__)
#       if (__STDC_VERSION__ > 100)
#           define BHO_LANG_STDC BHO_PREDEF_MAKE_YYYYMM(__STDC_VERSION__)
#       else
#           define BHO_LANG_STDC BHO_VERSION_NUMBER_AVAILABLE
#       endif
#   else
#       define BHO_LANG_STDC BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LANG_STDC
#   define BHO_LANG_STDC_AVAILABLE
#endif

#define BHO_LANG_STDC_NAME "Standard C"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_STDC,BHO_LANG_STDC_NAME)
