/*
Copyright Benjamin Worpitz 2018
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_LANGUAGE_CUDA_H
#define BHO_PREDEF_LANGUAGE_CUDA_H

#include <asio2/bho/predef/version_number.h>
#include <asio2/bho/predef/make.h>

/* tag::reference[]
= `BHO_LANG_CUDA`

https://en.wikipedia.org/wiki/CUDA[CUDA C/{CPP}] language.
If available, the version is detected as VV.RR.P.

[options="header"]
|===
| {predef_symbol} | {predef_version}

| `+__CUDACC__+` | {predef_detection}
| `+__CUDA__+` | {predef_detection}

| `CUDA_VERSION` | VV.RR.P
|===
*/ // end::reference[]

#define BHO_LANG_CUDA BHO_VERSION_NUMBER_NOT_AVAILABLE

#if defined(__CUDACC__) || defined(__CUDA__)
#   undef BHO_LANG_CUDA
#   include <cuda.h>
#   if defined(CUDA_VERSION)
#       define BHO_LANG_CUDA BHO_PREDEF_MAKE_10_VVRRP(CUDA_VERSION)
#   else
#       define BHO_LANG_CUDA BHO_VERSION_NUMBER_AVAILABLE
#   endif
#endif

#if BHO_LANG_CUDA
#   define BHO_LANG_CUDA_AVAILABLE
#endif

#define BHO_LANG_CUDA_NAME "CUDA C/C++"


#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_LANG_CUDA,BHO_LANG_CUDA_NAME)
