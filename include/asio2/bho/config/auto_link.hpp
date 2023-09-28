//  (C) Copyright John Maddock 2003.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

 /*
  *   LOCATION:    see http://www.boost.org for most recent version.
  *   FILE         auto_link.hpp
  *   VERSION      see <asio2/bho/version.hpp>
  *   DESCRIPTION: Automatic library inclusion for Borland/Microsoft compilers.
  */

/*************************************************************************

USAGE:
~~~~~~

Before including this header you must define one or more of define the following macros:

BHO_LIB_NAME:           Required: A string containing the basename of the library,
                          for example bho_regex.
BHO_LIB_TOOLSET:        Optional: the base name of the toolset.
BHO_DYN_LINK:           Optional: when set link to dll rather than static library.
BHO_LIB_DIAGNOSTIC:     Optional: when set the header will print out the name
                          of the library selected (useful for debugging).
BHO_AUTO_LINK_NOMANGLE: Specifies that we should link to BHO_LIB_NAME.lib,
                          rather than a mangled-name version.
BHO_AUTO_LINK_TAGGED:   Specifies that we link to libraries built with the --layout=tagged option.
                          This is essentially the same as the default name-mangled version, but without
                          the compiler name and version, or the Boost version.  Just the build options.
BHO_AUTO_LINK_SYSTEM:   Specifies that we link to libraries built with the --layout=system option.
                          This is essentially the same as the non-name-mangled version, but with
                          the prefix to differentiate static and dll builds

These macros will be undef'ed at the end of the header, further this header
has no include guards - so be sure to include it only once from your library!

Algorithm:
~~~~~~~~~~

Libraries for Borland and Microsoft compilers are automatically
selected here, the name of the lib is selected according to the following
formula:

BHO_LIB_PREFIX
   + BHO_LIB_NAME
   + "_"
   + BHO_LIB_TOOLSET
   + BHO_LIB_THREAD_OPT
   + BHO_LIB_RT_OPT
   + BHO_LIB_ARCH_AND_MODEL_OPT
   "-"
   + BHO_LIB_VERSION
   + BHO_LIB_SUFFIX

These are defined as:

BHO_LIB_PREFIX:     "lib" for static libraries otherwise "".

BHO_LIB_NAME:       The base name of the lib ( for example bho_regex).

BHO_LIB_TOOLSET:    The compiler toolset name (vc6, vc7, bcb5 etc).

BHO_LIB_THREAD_OPT: "-mt" for multithread builds, otherwise nothing.

BHO_LIB_RT_OPT:     A suffix that indicates the runtime library used,
                      contains one or more of the following letters after
                      a hyphen:

                      s      static runtime (dynamic if not present).
                      g      debug/diagnostic runtime (release if not present).
                      y      Python debug/diagnostic runtime (release if not present).
                      d      debug build (release if not present).
                      p      STLport build.
                      n      STLport build without its IOStreams.

BHO_LIB_ARCH_AND_MODEL_OPT: The architecture and address model
                              (-x32 or -x64 for x86/32 and x86/64 respectively)

BHO_LIB_VERSION:    The Boost version, in the form x_y, for Boost version x.y.

BHO_LIB_SUFFIX:     Static/import libraries extension (".lib", ".a") for the compiler.

***************************************************************************/

#ifdef __cplusplus
#  ifndef BHO_CONFIG_HPP
#     include <asio2/bho/config.hpp>
#  endif
#elif defined(_MSC_VER) && !defined(__MWERKS__) && !defined(__EDG_VERSION__)
//
// C language compatability (no, honestly)
//
#  define BHO_MSVC _MSC_VER
#  define BHO_STRINGIZE(X) BHO_DO_STRINGIZE(X)
#  define BHO_DO_STRINGIZE(X) #X
#endif
//
// Only include what follows for known and supported compilers:
//
#if defined(BHO_MSVC) \
    || defined(BHO_EMBTC_WINDOWS) \
    || defined(BHO_BORLANDC) \
    || (defined(__MWERKS__) && defined(_WIN32) && (__MWERKS__ >= 0x3000)) \
    || (defined(__ICL) && defined(_MSC_EXTENSIONS) && (_MSC_VER >= 1200)) \
    || (defined(BHO_CLANG) && defined(BHO_WINDOWS) && defined(_MSC_VER) && (__clang_major__ >= 4))

#ifndef BHO_VERSION_HPP
#  include <asio2/bho/version.hpp>
#endif

#ifndef BHO_LIB_NAME
#  error "Macro BHO_LIB_NAME not set (internal error)"
#endif

//
// error check:
//
#if defined(__MSVC_RUNTIME_CHECKS) && !defined(_DEBUG)
#  pragma message("Using the /RTC option without specifying a debug runtime will lead to linker errors")
#  pragma message("Hint: go to the code generation options and switch to one of the debugging runtimes")
#  error "Incompatible build options"
#endif
//
// select toolset if not defined already:
//
#ifndef BHO_LIB_TOOLSET
#  if defined(BHO_MSVC) && (BHO_MSVC < 1200)
    // Note: no compilers before 1200 are supported
#  elif defined(BHO_MSVC) && (BHO_MSVC < 1300)

#    ifdef UNDER_CE
       // eVC4:
#      define BHO_LIB_TOOLSET "evc4"
#    else
       // vc6:
#      define BHO_LIB_TOOLSET "vc6"
#    endif

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1310)

     // vc7:
#    define BHO_LIB_TOOLSET "vc7"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1400)

     // vc71:
#    define BHO_LIB_TOOLSET "vc71"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1500)

     // vc80:
#    define BHO_LIB_TOOLSET "vc80"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1600)

     // vc90:
#    define BHO_LIB_TOOLSET "vc90"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1700)

     // vc10:
#    define BHO_LIB_TOOLSET "vc100"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1800)

     // vc11:
#    define BHO_LIB_TOOLSET "vc110"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1900)

     // vc12:
#    define BHO_LIB_TOOLSET "vc120"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1910)

     // vc14:
#    define BHO_LIB_TOOLSET "vc140"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1920)

     // vc14.1:
#    define BHO_LIB_TOOLSET "vc141"

#  elif defined(BHO_MSVC) && (BHO_MSVC < 1930)

     // vc14.2:
#    define BHO_LIB_TOOLSET "vc142"

#  elif defined(BHO_MSVC)

     // vc14.3:
#    define BHO_LIB_TOOLSET "vc143"

#  elif defined(BHO_EMBTC_WINDOWS)

     // Embarcadero Clang based compilers:
#    define BHO_LIB_TOOLSET "embtc"

#  elif defined(BHO_BORLANDC)

     // CBuilder 6:
#    define BHO_LIB_TOOLSET "bcb"

#  elif defined(__ICL)

     // Intel C++, no version number:
#    define BHO_LIB_TOOLSET "iw"

#  elif defined(__MWERKS__) && (__MWERKS__ <= 0x31FF )

     // Metrowerks CodeWarrior 8.x
#    define BHO_LIB_TOOLSET "cw8"

#  elif defined(__MWERKS__) && (__MWERKS__ <= 0x32FF )

     // Metrowerks CodeWarrior 9.x
#    define BHO_LIB_TOOLSET "cw9"

#  elif defined(BHO_CLANG) && defined(BHO_WINDOWS) && defined(_MSC_VER) && (__clang_major__ >= 4)

     // Clang on Windows
#    define BHO_LIB_TOOLSET "clangw" BHO_STRINGIZE(__clang_major__)

#  endif
#endif // BHO_LIB_TOOLSET

//
// select thread opt:
//
#if defined(_MT) || defined(__MT__)
#  define BHO_LIB_THREAD_OPT "-mt"
#else
#  define BHO_LIB_THREAD_OPT
#endif

#if defined(_MSC_VER) || defined(__MWERKS__)

#  ifdef _DLL

#     if (defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)) && (defined(_STLP_OWN_IOSTREAMS) || defined(__STL_OWN_IOSTREAMS))

#        if defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-gydp"
#        elif defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))
#            define BHO_LIB_RT_OPT "-gdp"
#        elif defined(_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-gydp"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        elif defined(_DEBUG)
#            define BHO_LIB_RT_OPT "-gdp"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        else
#            define BHO_LIB_RT_OPT "-p"
#        endif

#     elif defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)

#        if defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-gydpn"
#        elif defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))
#            define BHO_LIB_RT_OPT "-gdpn"
#        elif defined(_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-gydpn"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        elif defined(_DEBUG)
#            define BHO_LIB_RT_OPT "-gdpn"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        else
#            define BHO_LIB_RT_OPT "-pn"
#        endif

#     else

#        if defined(_DEBUG) && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-gyd"
#        elif defined(_DEBUG)
#            define BHO_LIB_RT_OPT "-gd"
#        else
#            define BHO_LIB_RT_OPT
#        endif

#     endif

#  else

#     if (defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)) && (defined(_STLP_OWN_IOSTREAMS) || defined(__STL_OWN_IOSTREAMS))

#        if defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-sgydp"
#        elif defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))
#            define BHO_LIB_RT_OPT "-sgdp"
#        elif defined(_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#             define BHO_LIB_RT_OPT "-sgydp"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        elif defined(_DEBUG)
#             define BHO_LIB_RT_OPT "-sgdp"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        else
#            define BHO_LIB_RT_OPT "-sp"
#        endif

#     elif defined(__SGI_STL_PORT) || defined(_STLPORT_VERSION)

#        if defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#            define BHO_LIB_RT_OPT "-sgydpn"
#        elif defined(_DEBUG) && (defined(__STL_DEBUG) || defined(_STLP_DEBUG))
#            define BHO_LIB_RT_OPT "-sgdpn"
#        elif defined(_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#             define BHO_LIB_RT_OPT "-sgydpn"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        elif defined(_DEBUG)
#             define BHO_LIB_RT_OPT "-sgdpn"
#            pragma message("warning: STLport debug versions are built with /D_STLP_DEBUG=1")
#            error "Build options aren't compatible with pre-built libraries"
#        else
#            define BHO_LIB_RT_OPT "-spn"
#        endif

#     else

#        if defined(_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#             define BHO_LIB_RT_OPT "-sgyd"
#        elif defined(_DEBUG)
#             define BHO_LIB_RT_OPT "-sgd"
#        else
#            define BHO_LIB_RT_OPT "-s"
#        endif

#     endif

#  endif

#elif defined(BHO_EMBTC_WINDOWS)

#  ifdef _RTLDLL

#     if defined(_DEBUG)
#         define BHO_LIB_RT_OPT "-d"
#     else
#         define BHO_LIB_RT_OPT
#     endif

#  else

#     if defined(_DEBUG)
#         define BHO_LIB_RT_OPT "-sd"
#     else
#         define BHO_LIB_RT_OPT "-s"
#     endif

#  endif

#elif defined(BHO_BORLANDC)

//
// figure out whether we want the debug builds or not:
//
#if BHO_BORLANDC > 0x561
#pragma defineonoption BHO_BORLAND_DEBUG -v
#endif
//
// sanity check:
//
#if defined(__STL_DEBUG) || defined(_STLP_DEBUG)
#error "Pre-built versions of the Boost libraries are not provided in STLport-debug form"
#endif

#  ifdef _RTLDLL

#     if defined(BHO_BORLAND_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#         define BHO_LIB_RT_OPT "-yd"
#     elif defined(BHO_BORLAND_DEBUG)
#         define BHO_LIB_RT_OPT "-d"
#     elif defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#         define BHO_LIB_RT_OPT "-y"
#     else
#         define BHO_LIB_RT_OPT
#     endif

#  else

#     if defined(BHO_BORLAND_DEBUG)\
               && defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#         define BHO_LIB_RT_OPT "-syd"
#     elif defined(BHO_BORLAND_DEBUG)
#         define BHO_LIB_RT_OPT "-sd"
#     elif defined(BHO_DEBUG_PYTHON) && defined(BHO_LINKING_PYTHON)
#         define BHO_LIB_RT_OPT "-sy"
#     else
#         define BHO_LIB_RT_OPT "-s"
#     endif

#  endif

#endif

//
// BHO_LIB_ARCH_AND_MODEL_OPT
//

#if defined( _M_IX86 )
#  define BHO_LIB_ARCH_AND_MODEL_OPT "-x32"
#elif defined( _M_X64 )
#  define BHO_LIB_ARCH_AND_MODEL_OPT "-x64"
#elif defined( _M_ARM )
#  define BHO_LIB_ARCH_AND_MODEL_OPT "-a32"
#elif defined( _M_ARM64 )
#  define BHO_LIB_ARCH_AND_MODEL_OPT "-a64"
#endif

//
// select linkage opt:
//
#if (defined(_DLL) || defined(_RTLDLL)) && defined(BHO_DYN_LINK)
#  define BHO_LIB_PREFIX
#elif defined(BHO_DYN_LINK)
#  error "Mixing a dll boost library with a static runtime is a really bad idea..."
#else
#  define BHO_LIB_PREFIX "lib"
#endif

//
// now include the lib:
//
#if defined(BHO_LIB_NAME) \
      && defined(BHO_LIB_PREFIX) \
      && defined(BHO_LIB_TOOLSET) \
      && defined(BHO_LIB_THREAD_OPT) \
      && defined(BHO_LIB_RT_OPT) \
      && defined(BHO_LIB_ARCH_AND_MODEL_OPT) \
      && defined(BHO_LIB_VERSION)

#if defined(BHO_EMBTC_WIN64)
#  define BHO_LIB_SUFFIX ".a"
#else
#  define BHO_LIB_SUFFIX ".lib"
#endif

#ifdef BHO_AUTO_LINK_NOMANGLE
#  pragma comment(lib, BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_SUFFIX)
#  ifdef BHO_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_SUFFIX)
#  endif
#elif defined(BHO_AUTO_LINK_TAGGED)
#  pragma comment(lib, BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT BHO_LIB_SUFFIX)
#  ifdef BHO_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT BHO_LIB_SUFFIX)
#  endif
#elif defined(BHO_AUTO_LINK_SYSTEM)
#  pragma comment(lib, BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_SUFFIX)
#  ifdef BHO_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) BHO_LIB_SUFFIX)
#  endif
#elif defined(BHO_LIB_BUILDID)
#  pragma comment(lib, BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) "-" BHO_LIB_TOOLSET BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT "-" BHO_LIB_VERSION "-" BHO_STRINGIZE(BHO_LIB_BUILDID) BHO_LIB_SUFFIX)
#  ifdef BHO_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) "-" BHO_LIB_TOOLSET BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT "-" BHO_LIB_VERSION "-" BHO_STRINGIZE(BHO_LIB_BUILDID) BHO_LIB_SUFFIX)
#  endif
#else
#  pragma comment(lib, BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) "-" BHO_LIB_TOOLSET BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT "-" BHO_LIB_VERSION BHO_LIB_SUFFIX)
#  ifdef BHO_LIB_DIAGNOSTIC
#     pragma message ("Linking to lib file: " BHO_LIB_PREFIX BHO_STRINGIZE(BHO_LIB_NAME) "-" BHO_LIB_TOOLSET BHO_LIB_THREAD_OPT BHO_LIB_RT_OPT BHO_LIB_ARCH_AND_MODEL_OPT "-" BHO_LIB_VERSION BHO_LIB_SUFFIX)
#  endif
#endif

#else
#  error "some required macros where not defined (internal logic error)."
#endif


#endif // _MSC_VER || __BORLANDC__

//
// finally undef any macros we may have set:
//
#ifdef BHO_LIB_PREFIX
#  undef BHO_LIB_PREFIX
#endif
#if defined(BHO_LIB_NAME)
#  undef BHO_LIB_NAME
#endif
// Don't undef this one: it can be set by the user and should be the 
// same for all libraries:
//#if defined(BHO_LIB_TOOLSET)
//#  undef BHO_LIB_TOOLSET
//#endif
#if defined(BHO_LIB_THREAD_OPT)
#  undef BHO_LIB_THREAD_OPT
#endif
#if defined(BHO_LIB_RT_OPT)
#  undef BHO_LIB_RT_OPT
#endif
#if defined(BHO_LIB_ARCH_AND_MODEL_OPT)
#  undef BHO_LIB_ARCH_AND_MODEL_OPT
#endif
#if defined(BHO_LIB_LINK_OPT)
#  undef BHO_LIB_LINK_OPT
#endif
#if defined(BHO_LIB_DEBUG_OPT)
#  undef BHO_LIB_DEBUG_OPT
#endif
#if defined(BHO_DYN_LINK)
#  undef BHO_DYN_LINK
#endif
#if defined(BHO_LIB_SUFFIX)
#  undef BHO_LIB_SUFFIX
#endif
