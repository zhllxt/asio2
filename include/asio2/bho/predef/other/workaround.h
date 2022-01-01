/*
Copyright Rene Rivera 2017
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at
http://www.boost.org/LICENSE_1_0.txt)
*/

#ifndef BHO_PREDEF_WORKAROUND_H
#define BHO_PREDEF_WORKAROUND_H

/* tag::reference[]

= `BHO_PREDEF_WORKAROUND`

[source]
----
BHO_PREDEF_WORKAROUND(symbol,comp,major,minor,patch)
----

Usage:

[source]
----
#if BHO_PREDEF_WORKAROUND(BHO_COMP_CLANG,<,3,0,0)
    // Workaround for old clang compilers..
#endif
----

Defines a comparison against two version numbers that depends on the definion
of `BHO_STRICT_CONFIG`. When `BHO_STRICT_CONFIG` is defined this will expand
to a value convertible to `false`. Which has the effect of disabling all code
conditionally guarded by `BHO_PREDEF_WORKAROUND`. When `BHO_STRICT_CONFIG`
is undefine this expand to test the given `symbol` version value with the
`comp` comparison against `BHO_VERSION_NUMBER(major,minor,patch)`.

*/ // end::reference[]
#ifdef BHO_STRICT_CONFIG
#   define BHO_PREDEF_WORKAROUND(symbol, comp, major, minor, patch) (0)
#else
#   include <asio2/bho/predef/version_number.h>
#   define BHO_PREDEF_WORKAROUND(symbol, comp, major, minor, patch) \
        ( (symbol) != (0) ) && \
        ( (symbol) comp (BHO_VERSION_NUMBER( (major) , (minor) , (patch) )) )
#endif

/* tag::reference[]

= `BHO_PREDEF_TESTED_AT`

[source]
----
BHO_PREDEF_TESTED_AT(symbol,major,minor,patch)
----

Usage:

[source]
----
#if BHO_PREDEF_TESTED_AT(BHO_COMP_CLANG,3,5,0)
    // Needed for clang, and last checked for 3.5.0.
#endif
----

Defines a comparison against two version numbers that depends on the definion
of `BHO_STRICT_CONFIG` and `BHO_DETECT_OUTDATED_WORKAROUNDS`.
When `BHO_STRICT_CONFIG` is defined this will expand to a value convertible
to `false`. Which has the effect of disabling all code
conditionally guarded by `BHO_PREDEF_TESTED_AT`. When `BHO_STRICT_CONFIG`
is undefined this expand to either:

* A value convertible to `true` when `BHO_DETECT_OUTDATED_WORKAROUNDS` is not
  defined.
* A value convertible `true` when the expansion of
  `BHO_PREDEF_WORKAROUND(symbol, <=, major, minor, patch)` is `true` and
  `BHO_DETECT_OUTDATED_WORKAROUNDS` is defined.
* A compile error when the expansion of
  `BHO_PREDEF_WORKAROUND(symbol, >, major, minor, patch)` is true and
  `BHO_DETECT_OUTDATED_WORKAROUNDS` is defined.

*/ // end::reference[]
#ifdef BHO_STRICT_CONFIG
#   define BHO_PREDEF_TESTED_AT(symbol, major, minor, patch) (0)
#else
#   ifdef BHO_DETECT_OUTDATED_WORKAROUNDS
#       define BHO_PREDEF_TESTED_AT(symbol, major, minor, patch) ( \
            BHO_PREDEF_WORKAROUND(symbol, <=, major, minor, patch) \
            ? 1 \
            : (1%0) )
#   else
#       define BHO_PREDEF_TESTED_AT(symbol, major, minor, patch) \
            ( (symbol) >= BHO_VERSION_NUMBER_AVAILABLE )
#   endif
#endif

#endif
