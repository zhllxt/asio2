//  (C) Copyright John Maddock 2001-8.
//  (C) Copyright Peter Dimov 2001.
//  (C) Copyright Jens Maurer 2001.
//  (C) Copyright David Abrahams 2002 - 2003.
//  (C) Copyright Aleksey Gurtovoy 2002 - 2003.
//  (C) Copyright Guillaume Melquiond 2002 - 2003.
//  (C) Copyright Beman Dawes 2003.
//  (C) Copyright Martin Wille 2003.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Intel compiler setup:

#if defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1500) && (defined(_MSC_VER) || defined(__GNUC__))

#ifdef _MSC_VER

#include <asio2/bho/config/compiler/visualc.hpp>

#undef BHO_MSVC
#undef BHO_MSVC_FULL_VER

#if (__INTEL_COMPILER >= 1500) && (_MSC_VER >= 1900)
//
// These appear to be supported, even though VC++ may not support them:
//
#define BHO_HAS_EXPM1
#define BHO_HAS_LOG1P
#undef BHO_NO_CXX14_BINARY_LITERALS
// This one may be a little risky to enable??
#undef BHO_NO_SFINAE_EXPR

#endif

#if (__INTEL_COMPILER <= 1600) && !defined(BHO_NO_CXX14_VARIABLE_TEMPLATES)
#  define BHO_NO_CXX14_VARIABLE_TEMPLATES
#endif

#else // defined(_MSC_VER)

#include <asio2/bho/config/compiler/gcc.hpp>

#undef BHO_GCC_VERSION
#undef BHO_GCC_CXX11
#undef BHO_GCC
#undef BHO_FALLTHROUGH

// Broken in all versions up to 17 (newer versions not tested)
#if (__INTEL_COMPILER <= 1700) && !defined(BHO_NO_CXX14_CONSTEXPR)
#  define BHO_NO_CXX14_CONSTEXPR
#endif

#if (__INTEL_COMPILER >= 1800) && (__cplusplus >= 201703)
#  define BHO_FALLTHROUGH [[fallthrough]]
#endif

#endif // defined(_MSC_VER)

#undef BHO_COMPILER

#if defined(__INTEL_COMPILER)
#if __INTEL_COMPILER == 9999
#  define BHO_INTEL_CXX_VERSION 1200 // Intel bug in 12.1.
#else
#  define BHO_INTEL_CXX_VERSION __INTEL_COMPILER
#endif
#elif defined(__ICL)
#  define BHO_INTEL_CXX_VERSION __ICL
#elif defined(__ICC)
#  define BHO_INTEL_CXX_VERSION __ICC
#elif defined(__ECC)
#  define BHO_INTEL_CXX_VERSION __ECC
#endif

// Flags determined by comparing output of 'icpc -dM -E' with and without '-std=c++0x'
#if (!(defined(_WIN32) || defined(_WIN64)) && defined(__STDC_HOSTED__) && (__STDC_HOSTED__ && (BHO_INTEL_CXX_VERSION <= 1200))) || defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#  define BHO_INTEL_STDCXX0X
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#  define BHO_INTEL_STDCXX0X
#endif

#ifdef __GNUC__
#  define BHO_INTEL_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#if !defined(BHO_COMPILER)
#  if defined(BHO_INTEL_STDCXX0X)
#    define BHO_COMPILER "Intel C++ C++0x mode version " BHO_STRINGIZE(BHO_INTEL_CXX_VERSION)
#  else
#    define BHO_COMPILER "Intel C++ version " BHO_STRINGIZE(BHO_INTEL_CXX_VERSION)
#  endif
#endif

#define BHO_INTEL BHO_INTEL_CXX_VERSION

#if defined(_WIN32) || defined(_WIN64)
#  define BHO_INTEL_WIN BHO_INTEL
#else
#  define BHO_INTEL_LINUX BHO_INTEL
#endif

#else // defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1500) && (defined(_MSC_VER) || defined(__GNUC__))

#include <asio2/bho/config/compiler/common_edg.hpp>

#if defined(__INTEL_COMPILER)
#if __INTEL_COMPILER == 9999
#  define BHO_INTEL_CXX_VERSION 1200 // Intel bug in 12.1.
#else
#  define BHO_INTEL_CXX_VERSION __INTEL_COMPILER
#endif
#elif defined(__ICL)
#  define BHO_INTEL_CXX_VERSION __ICL
#elif defined(__ICC)
#  define BHO_INTEL_CXX_VERSION __ICC
#elif defined(__ECC)
#  define BHO_INTEL_CXX_VERSION __ECC
#endif

// Flags determined by comparing output of 'icpc -dM -E' with and without '-std=c++0x'
#if (!(defined(_WIN32) || defined(_WIN64)) && defined(__STDC_HOSTED__) && (__STDC_HOSTED__ && (BHO_INTEL_CXX_VERSION <= 1200))) || defined(__GXX_EXPERIMENTAL_CPP0X__) || defined(__GXX_EXPERIMENTAL_CXX0X__)
#  define BHO_INTEL_STDCXX0X
#endif
#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#  define BHO_INTEL_STDCXX0X
#endif

#ifdef __GNUC__
#  define BHO_INTEL_GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#if !defined(BHO_COMPILER)
#  if defined(BHO_INTEL_STDCXX0X)
#    define BHO_COMPILER "Intel C++ C++0x mode version " BHO_STRINGIZE(BHO_INTEL_CXX_VERSION)
#  else
#    define BHO_COMPILER "Intel C++ version " BHO_STRINGIZE(BHO_INTEL_CXX_VERSION)
#  endif
#endif

#define BHO_INTEL BHO_INTEL_CXX_VERSION

#if defined(_WIN32) || defined(_WIN64)
#  define BHO_INTEL_WIN BHO_INTEL
#else
#  define BHO_INTEL_LINUX BHO_INTEL
#endif

#if (BHO_INTEL_CXX_VERSION <= 600)

#  if defined(_MSC_VER) && (_MSC_VER <= 1300) // added check for <= VC 7 (Peter Dimov)

// Boost libraries assume strong standard conformance unless otherwise
// indicated by a config macro. As configured by Intel, the EDG front-end
// requires certain compiler options be set to achieve that strong conformance.
// Particularly /Qoption,c,--arg_dep_lookup (reported by Kirk Klobe & Thomas Witt)
// and /Zc:wchar_t,forScope. See boost-root/tools/build/intel-win32-tools.jam for
// details as they apply to particular versions of the compiler. When the
// compiler does not predefine a macro indicating if an option has been set,
// this config file simply assumes the option has been set.
// Thus BHO_NO_ARGUMENT_DEPENDENT_LOOKUP will not be defined, even if
// the compiler option is not enabled.

#     define BHO_NO_SWPRINTF
#  endif

// Void returns, 64 bit integrals don't work when emulating VC 6 (Peter Dimov)

#  if defined(_MSC_VER) && (_MSC_VER <= 1200)
#     define BHO_NO_VOID_RETURNS
#     define BHO_NO_INTEGRAL_INT64_T
#  endif

#endif

#if (BHO_INTEL_CXX_VERSION <= 710) && defined(_WIN32)
#  define BHO_NO_POINTER_TO_MEMBER_TEMPLATE_PARAMETERS
#endif

// See http://aspn.activestate.com/ASPN/Mail/Message/bho/1614864
#if BHO_INTEL_CXX_VERSION < 600
#  define BHO_NO_INTRINSIC_WCHAR_T
#else
// We should test the macro _WCHAR_T_DEFINED to check if the compiler
// supports wchar_t natively. *BUT* there is a problem here: the standard
// headers define this macro if they typedef wchar_t. Anyway, we're lucky
// because they define it without a value, while Intel C++ defines it
// to 1. So we can check its value to see if the macro was defined natively
// or not.
// Under UNIX, the situation is exactly the same, but the macro _WCHAR_T
// is used instead.
#  if ((_WCHAR_T_DEFINED + 0) == 0) && ((_WCHAR_T + 0) == 0)
#    define BHO_NO_INTRINSIC_WCHAR_T
#  endif
#endif

#if defined(__GNUC__) && !defined(BHO_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL)
//
// Figure out when Intel is emulating this gcc bug
// (All Intel versions prior to 9.0.26, and versions
// later than that if they are set up to emulate gcc 3.2
// or earlier):
//
#  if ((__GNUC__ == 3) && (__GNUC_MINOR__ <= 2)) || (BHO_INTEL < 900) || (__INTEL_COMPILER_BUILD_DATE < 20050912)
#     define BHO_FUNCTION_SCOPE_USING_DECLARATION_BREAKS_ADL
#  endif
#endif
#if (defined(__GNUC__) && (__GNUC__ < 4)) || (defined(_WIN32) && (BHO_INTEL_CXX_VERSION <= 1200)) || (BHO_INTEL_CXX_VERSION <= 1200)
// GCC or VC emulation:
#define BHO_NO_TWO_PHASE_NAME_LOOKUP
#endif
//
// Verify that we have actually got BHO_NO_INTRINSIC_WCHAR_T
// set correctly, if we don't do this now, we will get errors later
// in type_traits code among other things, getting this correct
// for the Intel compiler is actually remarkably fragile and tricky:
//
#ifdef __cplusplus
#if defined(BHO_NO_INTRINSIC_WCHAR_T)
#include <cwchar>
template< typename T > struct assert_no_intrinsic_wchar_t;
template<> struct assert_no_intrinsic_wchar_t<wchar_t> { typedef void type; };
// if you see an error here then you need to unset BHO_NO_INTRINSIC_WCHAR_T
// where it is defined above:
typedef assert_no_intrinsic_wchar_t<unsigned short>::type assert_no_intrinsic_wchar_t_;
#else
template< typename T > struct assert_intrinsic_wchar_t;
template<> struct assert_intrinsic_wchar_t<wchar_t> {};
// if you see an error here then define BHO_NO_INTRINSIC_WCHAR_T on the command line:
template<> struct assert_intrinsic_wchar_t<unsigned short> {};
#endif
#endif

#if defined(_MSC_VER) && (_MSC_VER+0 >= 1000)
#  if _MSC_VER >= 1200
#     define BHO_HAS_MS_INT64
#  endif
#  define BHO_NO_SWPRINTF
#  define BHO_NO_TWO_PHASE_NAME_LOOKUP
#elif defined(_WIN32)
#  define BHO_DISABLE_WIN32
#endif

// I checked version 6.0 build 020312Z, it implements the NRVO.
// Correct this as you find out which version of the compiler
// implemented the NRVO first.  (Daniel Frey)
#if (BHO_INTEL_CXX_VERSION >= 600)
#  define BHO_HAS_NRVO
#endif

// Branch prediction hints
// I'm not sure 8.0 was the first version to support these builtins,
// update the condition if the version is not accurate. (Andrey Semashev)
#if defined(__GNUC__) && BHO_INTEL_CXX_VERSION >= 800
#define BHO_LIKELY(x) __builtin_expect(x, 1)
#define BHO_UNLIKELY(x) __builtin_expect(x, 0)
#endif

// RTTI
// __RTTI is the EDG macro
// __INTEL_RTTI__ is the Intel macro
// __GXX_RTTI is the g++ macro
// _CPPRTTI is the MSVC++ macro
#if !defined(__RTTI) && !defined(__INTEL_RTTI__) && !defined(__GXX_RTTI) && !defined(_CPPRTTI)

#if !defined(BHO_NO_RTTI)
# define BHO_NO_RTTI
#endif

// in MS mode, static typeid works even when RTTI is off
#if !defined(_MSC_VER) && !defined(BHO_NO_TYPEID)
# define BHO_NO_TYPEID
#endif

#endif

//
// versions check:
// we don't support Intel prior to version 6.0:
#if BHO_INTEL_CXX_VERSION < 600
#  error "Compiler not supported or configured - please reconfigure"
#endif

// Intel on MacOS requires
#if defined(__APPLE__) && defined(__INTEL_COMPILER)
#  define BHO_NO_TWO_PHASE_NAME_LOOKUP
#endif

// Intel on Altix Itanium
#if defined(__itanium__) && defined(__INTEL_COMPILER)
#  define BHO_NO_TWO_PHASE_NAME_LOOKUP
#endif

//
// An attempt to value-initialize a pointer-to-member may trigger an
// internal error on Intel <= 11.1 (last checked version), as was
// reported by John Maddock, Intel support issue 589832, May 2010.
// Moreover, according to test results from Huang-Vista-x86_32_intel,
// intel-vc9-win-11.1 may leave a non-POD array uninitialized, in some
// cases when it should be value-initialized.
// (Niels Dekker, LKEB, May 2010)
// Apparently Intel 12.1 (compiler version number 9999 !!) has the same issue (compiler regression).
#if defined(__INTEL_COMPILER)
#  if (__INTEL_COMPILER <= 1110) || (__INTEL_COMPILER == 9999) || (defined(_WIN32) && (__INTEL_COMPILER < 1600))
#    define BHO_NO_COMPLETE_VALUE_INITIALIZATION
#  endif
#endif

//
// Dynamic shared object (DSO) and dynamic-link library (DLL) support
//
#if defined(__GNUC__) && (__GNUC__ >= 4)
#  define BHO_SYMBOL_EXPORT __attribute__((visibility("default")))
#  define BHO_SYMBOL_IMPORT
#  define BHO_SYMBOL_VISIBLE __attribute__((visibility("default")))
#endif

// Type aliasing hint
#if defined(__GNUC__) && (BHO_INTEL_CXX_VERSION >= 1300)
#  define BHO_MAY_ALIAS __attribute__((__may_alias__))
#endif

//
// C++0x features
// For each feature we need to check both the Intel compiler version, 
// and the version of MSVC or GCC that we are emulating.
// See http://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler/
// for a list of which features were implemented in which Intel releases.
//
#if defined(BHO_INTEL_STDCXX0X)
// BHO_NO_CXX11_CONSTEXPR:
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40600)) && !defined(_MSC_VER)
// Available in earlier Intel versions, but fail our tests:
#  undef BHO_NO_CXX11_CONSTEXPR
#endif
// BHO_NO_CXX11_NULLPTR:
#if (BHO_INTEL_CXX_VERSION >= 1210) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40600)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_NULLPTR
#endif
// BHO_NO_CXX11_TEMPLATE_ALIASES
#if (BHO_INTEL_CXX_VERSION >= 1210) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40700)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_TEMPLATE_ALIASES
#endif

// BHO_NO_CXX11_DECLTYPE
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40300)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_DECLTYPE
#endif

// BHO_NO_CXX11_DECLTYPE_N3276
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40800)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_DECLTYPE_N3276
#endif

// BHO_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40300)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#endif

// BHO_NO_CXX11_RVALUE_REFERENCES
#if (BHO_INTEL_CXX_VERSION >= 1300) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40300)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
// This is available from earlier Intel versions, but breaks Filesystem and other libraries:
#  undef BHO_NO_CXX11_RVALUE_REFERENCES
#endif

// BHO_NO_CXX11_STATIC_ASSERT
#if (BHO_INTEL_CXX_VERSION >= 1110) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40300)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_STATIC_ASSERT
#endif

// BHO_NO_CXX11_VARIADIC_TEMPLATES
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_VARIADIC_TEMPLATES
#endif

// BHO_NO_CXX11_VARIADIC_MACROS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40200)) && (!defined(_MSC_VER) || (_MSC_VER >= 1400))
#  undef BHO_NO_CXX11_VARIADIC_MACROS
#endif

// BHO_NO_CXX11_AUTO_DECLARATIONS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_AUTO_DECLARATIONS
#endif

// BHO_NO_CXX11_AUTO_MULTIDECLARATIONS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_AUTO_MULTIDECLARATIONS
#endif

// BHO_NO_CXX11_CHAR16_T
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
#  undef BHO_NO_CXX11_CHAR16_T
#endif

// BHO_NO_CXX11_CHAR32_T
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
#  undef BHO_NO_CXX11_CHAR32_T
#endif

// BHO_NO_CXX11_DEFAULTED_FUNCTIONS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_DEFAULTED_FUNCTIONS
#endif

// BHO_NO_CXX11_DELETED_FUNCTIONS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_DELETED_FUNCTIONS
#endif

// BHO_NO_CXX11_HDR_INITIALIZER_LIST
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_VER >= 1700))
#  undef BHO_NO_CXX11_HDR_INITIALIZER_LIST
#endif

// BHO_NO_CXX11_SCOPED_ENUMS
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40501)) && (!defined(_MSC_VER) || (_MSC_VER >= 1700))
// This is available but broken in earlier Intel releases.
#  undef BHO_NO_CXX11_SCOPED_ENUMS
#endif

// BHO_NO_SFINAE_EXPR
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
#  undef BHO_NO_SFINAE_EXPR
#endif

// BHO_NO_CXX11_SFINAE_EXPR
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40800)) && !defined(_MSC_VER)
#  undef BHO_NO_CXX11_SFINAE_EXPR
#endif

// BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
// This is available in earlier Intel releases, but breaks Multiprecision:
#  undef BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#endif

// BHO_NO_CXX11_LAMBDAS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500)) && (!defined(_MSC_VER) || (_MSC_VER >= 1600))
#  undef BHO_NO_CXX11_LAMBDAS
#endif

// BHO_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500))
#  undef BHO_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#endif

// BHO_NO_CXX11_RANGE_BASED_FOR
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40600)) && (!defined(_MSC_VER) || (_MSC_VER >= 1700))
#  undef BHO_NO_CXX11_RANGE_BASED_FOR
#endif

// BHO_NO_CXX11_RAW_LITERALS
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_RAW_LITERALS
#endif

// BHO_NO_CXX11_UNICODE_LITERALS
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40500)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
#  undef BHO_NO_CXX11_UNICODE_LITERALS
#endif

// BHO_NO_CXX11_NOEXCEPT
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40600)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
// Available in earlier Intel release, but generates errors when used with 
// conditional exception specifications, for example in multiprecision:
#  undef BHO_NO_CXX11_NOEXCEPT
#endif

// BHO_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40600)) && (!defined(_MSC_VER) || (_MSC_VER >= 9999))
#  undef BHO_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#endif

// BHO_NO_CXX11_USER_DEFINED_LITERALS
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40700)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 190021730))
#  undef BHO_NO_CXX11_USER_DEFINED_LITERALS
#endif

// BHO_NO_CXX11_ALIGNAS
#if (BHO_INTEL_CXX_VERSION >= 1500) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40800)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 190021730))
#  undef BHO_NO_CXX11_ALIGNAS
#  undef BHO_NO_CXX11_ALIGNOF
#endif

// BHO_NO_CXX11_TRAILING_RESULT_TYPES
#if (BHO_INTEL_CXX_VERSION >= 1200) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 180020827))
#  undef BHO_NO_CXX11_TRAILING_RESULT_TYPES
#endif

// BHO_NO_CXX11_INLINE_NAMESPACES
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40400)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 190021730))
#  undef BHO_NO_CXX11_INLINE_NAMESPACES
#endif

// BHO_NO_CXX11_REF_QUALIFIERS
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40800)) && (!defined(_MSC_VER) || (_MSC_FULL_VER >= 190021730))
#  undef BHO_NO_CXX11_REF_QUALIFIERS
#endif

// BHO_NO_CXX11_FINAL
// BHO_NO_CXX11_OVERRIDE
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 40700)) && (!defined(_MSC_VER) || (_MSC_VER >= 1700))
#  undef BHO_NO_CXX11_FINAL
#  undef BHO_NO_CXX11_OVERRIDE
#endif

// BHO_NO_CXX11_UNRESTRICTED_UNION
#if (BHO_INTEL_CXX_VERSION >= 1400) && (!defined(BHO_INTEL_GCC_VERSION) || (BHO_INTEL_GCC_VERSION >= 50100)) && (!defined(_MSC_VER))
#  undef BHO_NO_CXX11_UNRESTRICTED_UNION
#endif

#endif // defined(BHO_INTEL_STDCXX0X)

//
// Broken in all versions up to 15:
#define BHO_NO_CXX11_FIXED_LENGTH_VARIADIC_TEMPLATE_EXPANSION_PACKS

#if defined(BHO_INTEL_STDCXX0X) && (BHO_INTEL_CXX_VERSION <= 1310)
#  define BHO_NO_CXX11_HDR_FUTURE
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#endif

#if defined(BHO_INTEL_STDCXX0X) && (BHO_INTEL_CXX_VERSION == 1400)
// A regression in Intel's compiler means that <tuple> seems to be broken in this release as well as <future> :
#  define BHO_NO_CXX11_HDR_FUTURE
#  define BHO_NO_CXX11_HDR_TUPLE
#endif

#if (BHO_INTEL_CXX_VERSION < 1200)
//
// fenv.h appears not to work with Intel prior to 12.0:
//
#  define BHO_NO_FENV_H
#endif

// Intel 13.10 fails to access defaulted functions of a base class declared in private or protected sections,
// producing the following errors:
// error #453: protected function "..." (declared at ...") is not accessible through a "..." pointer or object
#if (BHO_INTEL_CXX_VERSION <= 1310)
#  define BHO_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#  define BHO_HAS_STDINT_H
#endif

#if defined(__CUDACC__)
#  if defined(BHO_GCC_CXX11)
#    define BHO_NVCC_CXX11
#  else
#    define BHO_NVCC_CXX03
#  endif
#endif

#if defined(__LP64__) && defined(__GNUC__) && (BHO_INTEL_CXX_VERSION >= 1310) && !defined(BHO_NVCC_CXX03)
#  define BHO_HAS_INT128
#endif

#endif // defined(__INTEL_COMPILER) && (__INTEL_COMPILER >= 1500) && (defined(_MSC_VER) || defined(__GNUC__))
//
// last known and checked version:
#if (BHO_INTEL_CXX_VERSION > 1700)
#  if defined(BHO_ASSERT_CONFIG)
#     error "BHO.Config is older than your compiler - please check for an updated Boost release."
#  elif defined(_MSC_VER)
//
//      We don't emit this warning any more, since we have so few
//      defect macros set anyway (just the one).
//
//#     pragma message("boost: Unknown compiler version - please run the configure tests and report the results")
#  endif
#endif

