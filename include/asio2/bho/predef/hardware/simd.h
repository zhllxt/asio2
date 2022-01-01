/*
Copyright Charly Chevalier 2015
Copyright Joel Falcou 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#include <asio2/bho/predef/hardware/simd/x86.h>
#include <asio2/bho/predef/hardware/simd/x86_amd.h>
#include <asio2/bho/predef/hardware/simd/arm.h>
#include <asio2/bho/predef/hardware/simd/ppc.h>

#ifndef BHO_PREDEF_HARDWARE_SIMD_H
#define BHO_PREDEF_HARDWARE_SIMD_H

#include <asio2/bho/predef/version_number.h>

/* tag::reference[]
= Using the `BHO_HW_SIMD_*` predefs

SIMD predefs depend on compiler options. For example, you will have to add the
option `-msse3` to clang or gcc to enable SSE3. SIMD predefs are also inclusive.
This means that if SSE3 is enabled, then every other extensions with a lower
version number will implicitly be enabled and detected. However, some extensions
are CPU specific, they may not be detected nor enabled when an upper version is
enabled.

NOTE: SSE(1) and SSE2 are automatically enabled by default when using x86-64
architecture.

To check if any SIMD extension has been enabled, you can use:

[source]
----
#include <asio2/bho/predef/hardware/simd.h>
#include <iostream>

int main()
{
#if defined(BHO_HW_SIMD_AVAILABLE)
    std::cout << "SIMD detected!" << std::endl;
#endif
    return 0;
}
----

When writing SIMD specific code, you may want to check if a particular extension
has been detected. To do so you have to use the right architecture predef and
compare it. Those predef are of the form `BHO_HW_SIMD_"ARCH"` (where `"ARCH"`
is either `ARM`, `PPC`, or `X86`). For example, if you compile code for x86
architecture, you will have to use `BHO_HW_SIMD_X86`. Its value will be the
version number of the most recent SIMD extension detected for the architecture.

To check if an extension has been enabled:

[source]
----
#include <asio2/bho/predef/hardware/simd.h>
#include <iostream>

int main()
{
#if BHO_HW_SIMD_X86 >= BHO_HW_SIMD_X86_SSE3_VERSION
    std::cout << "This is SSE3!" << std::endl;
#endif
    return 0;
}
----

NOTE: The *_VERSION* defines that map version number to actual real
identifiers. This way it is easier to write comparisons without messing up with
version numbers.

To *"strictly"* check the most recent detected extension:

[source]
----
#include <asio2/bho/predef/hardware/simd.h>
#include <iostream>

int main()
{
#if BHO_HW_SIMD_X86 == BHO_HW_SIMD_X86_SSE3_VERSION
    std::cout << "This is SSE3 and this is the most recent enabled extension!"
        << std::endl;
#endif
    return 0;
}
----

Because of the version systems of predefs and of the inclusive property of SIMD
extensions macros, you can easily check for ranges of supported extensions:

[source]
----
#include <asio2/bho/predef/hardware/simd.h>
#include <iostream>

int main()
{
#if BHO_HW_SIMD_X86 >= BHO_HW_SIMD_X86_SSE2_VERSION &&\
    BHO_HW_SIMD_X86 <= BHO_HW_SIMD_X86_SSSE3_VERSION
    std::cout << "This is SSE2, SSE3 and SSSE3!" << std::endl;
#endif
    return 0;
}
----

NOTE: Unlike gcc and clang, Visual Studio does not allow you to specify precisely
the SSE variants you want to use, the only detections that will take place are
SSE, SSE2, AVX and AVX2. For more informations,
    see [@https://msdn.microsoft.com/en-us/library/b0084kay.aspx here].


*/ // end::reference[]

// We check if SIMD extension of multiples architectures have been detected,
// if yes, then this is an error!
//
// NOTE: _X86_AMD implies _X86, so there is no need to check for it here!
//
#if defined(BHO_HW_SIMD_ARM_AVAILABLE) && defined(BHO_HW_SIMD_PPC_AVAILABLE) ||\
    defined(BHO_HW_SIMD_ARM_AVAILABLE) && defined(BHO_HW_SIMD_X86_AVAILABLE) ||\
    defined(BHO_HW_SIMD_PPC_AVAILABLE) && defined(BHO_HW_SIMD_X86_AVAILABLE)
#   error "Multiple SIMD architectures detected, this cannot happen!"
#endif

#if defined(BHO_HW_SIMD_X86_AVAILABLE) && defined(BHO_HW_SIMD_X86_AMD_AVAILABLE)
    // If both standard _X86 and _X86_AMD are available,
    // then take the biggest version of the two!
#   if BHO_HW_SIMD_X86 >= BHO_HW_SIMD_X86_AMD
#      define BHO_HW_SIMD BHO_HW_SIMD_X86
#   else
#      define BHO_HW_SIMD BHO_HW_SIMD_X86_AMD
#   endif
#endif

#if !defined(BHO_HW_SIMD)
    // At this point, only one of these two is defined
#   if defined(BHO_HW_SIMD_X86_AVAILABLE)
#      define BHO_HW_SIMD BHO_HW_SIMD_X86
#   endif
#   if defined(BHO_HW_SIMD_X86_AMD_AVAILABLE)
#      define BHO_HW_SIMD BHO_HW_SIMD_X86_AMD
#   endif
#endif

#if defined(BHO_HW_SIMD_ARM_AVAILABLE)
#   define BHO_HW_SIMD BHO_HW_SIMD_ARM
#endif

#if defined(BHO_HW_SIMD_PPC_AVAILABLE)
#   define BHO_HW_SIMD BHO_HW_SIMD_PPC
#endif

#if defined(BHO_HW_SIMD)
#   define BHO_HW_SIMD_AVAILABLE
#else
#   define BHO_HW_SIMD BHO_VERSION_NUMBER_NOT_AVAILABLE
#endif

#define BHO_HW_SIMD_NAME "Hardware SIMD"

#endif

#include <asio2/bho/predef/detail/test.h>
BHO_PREDEF_DECLARE_TEST(BHO_HW_SIMD, BHO_HW_SIMD_NAME)
