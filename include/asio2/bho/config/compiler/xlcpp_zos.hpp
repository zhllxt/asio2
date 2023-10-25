//  Copyright (c) 2017 Dynatrace
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt

//  See http://www.boost.org for most recent version.

//  Compiler setup for IBM z/OS XL C/C++ compiler.

// Oldest compiler version currently supported is 2.1 (V2R1)
#if !defined(__IBMCPP__) || !defined(__COMPILER_VER__) || __COMPILER_VER__ < 0x42010000
#  error "Compiler not supported or configured - please reconfigure"
#endif

#if __COMPILER_VER__ > 0x42010000
#  if defined(BHO_ASSERT_CONFIG)
#     error "Unknown compiler version - please run the configure tests and report the results"
#  endif
#endif

#define BHO_COMPILER "IBM z/OS XL C/C++ version " BHO_STRINGIZE(__COMPILER_VER__)
#define BHO_XLCPP_ZOS __COMPILER_VER__

// -------------------------------------

#include <features.h> // For __UU, __C99, __TR1, ...

#if !defined(__IBMCPP_DEFAULTED_AND_DELETED_FUNCTIONS)
#  define BHO_NO_CXX11_DELETED_FUNCTIONS
#  define BHO_NO_CXX11_DEFAULTED_FUNCTIONS
#  define BHO_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS
#endif

// -------------------------------------

#if defined(__UU) || defined(__C99) || defined(__TR1)
#  define BHO_HAS_LOG1P
#  define BHO_HAS_EXPM1
#endif

#if defined(__C99) || defined(__TR1)
#  define BHO_HAS_STDINT_H
#else
#  define BHO_NO_FENV_H
#endif

// -------------------------------------

#define BHO_HAS_NRVO

#if !defined(__RTTI_ALL__)
#  define BHO_NO_RTTI
#endif

#if !defined(_CPPUNWIND) && !defined(__EXCEPTIONS)
#  define BHO_NO_EXCEPTIONS
#endif

#if defined(_LONG_LONG) || defined(__IBMCPP_C99_LONG_LONG) || defined(__LL)
#  define BHO_HAS_LONG_LONG
#else
#  define BHO_NO_LONG_LONG
#endif

#if defined(_LONG_LONG) || defined(__IBMCPP_C99_LONG_LONG) || defined(__LL) || defined(_LP64)
#  define BHO_HAS_MS_INT64
#endif

#define BHO_NO_SFINAE_EXPR
#define BHO_NO_CXX11_SFINAE_EXPR

#if defined(__IBMCPP_VARIADIC_TEMPLATES)
#  define BHO_HAS_VARIADIC_TMPL
#else
#  define BHO_NO_CXX11_VARIADIC_TEMPLATES
#  define BHO_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#endif

#if defined(__IBMCPP_STATIC_ASSERT)
#  define BHO_HAS_STATIC_ASSERT
#else
#  define BHO_NO_CXX11_STATIC_ASSERT
#endif

#if defined(__IBMCPP_RVALUE_REFERENCES)
#  define BHO_HAS_RVALUE_REFS
#else
#  define BHO_NO_CXX11_RVALUE_REFERENCES
#endif

#if !defined(__IBMCPP_SCOPED_ENUM)
#  define BHO_NO_CXX11_SCOPED_ENUMS
#endif

#define BHO_NO_CXX11_FIXED_LENGTH_VARIADIC_TEMPLATE_EXPANSION_PACKS
#define BHO_NO_CXX11_TEMPLATE_ALIASES
#define BHO_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS

#if !defined(__IBMCPP_EXPLICIT_CONVERSION_OPERATORS)
#  define BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#endif

#if !defined(__IBMCPP_DECLTYPE)
#  define BHO_NO_CXX11_DECLTYPE
#else
#  define BHO_HAS_DECLTYPE
#endif
#define BHO_NO_CXX11_DECLTYPE_N3276

#if !defined(__IBMCPP_INLINE_NAMESPACE)
#  define BHO_NO_CXX11_INLINE_NAMESPACES
#endif

#if !defined(__IBMCPP_AUTO_TYPEDEDUCTION)
#  define BHO_NO_CXX11_AUTO_MULTIDECLARATIONS
#  define BHO_NO_CXX11_AUTO_DECLARATIONS
#  define BHO_NO_CXX11_TRAILING_RESULT_TYPES
#endif

#if !defined(__IBM_CHAR32_T__)
#  define BHO_NO_CXX11_CHAR32_T
#endif
#if !defined(__IBM_CHAR16_T__)
#  define BHO_NO_CXX11_CHAR16_T
#endif

#if !defined(__IBMCPP_CONSTEXPR)
#  define BHO_NO_CXX11_CONSTEXPR
#endif

#define BHO_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define BHO_NO_CXX11_UNICODE_LITERALS
#define BHO_NO_CXX11_RAW_LITERALS
#define BHO_NO_CXX11_RANGE_BASED_FOR
#define BHO_NO_CXX11_NULLPTR
#define BHO_NO_CXX11_NOEXCEPT
#define BHO_NO_CXX11_LAMBDAS
#define BHO_NO_CXX11_USER_DEFINED_LITERALS
#define BHO_NO_CXX11_THREAD_LOCAL
#define BHO_NO_CXX11_REF_QUALIFIERS
#define BHO_NO_CXX11_FINAL
#define BHO_NO_CXX11_OVERRIDE
#define BHO_NO_CXX11_ALIGNAS
#define BHO_NO_CXX11_ALIGNOF
#define BHO_NO_CXX11_UNRESTRICTED_UNION
#define BHO_NO_CXX14_VARIABLE_TEMPLATES
#define BHO_NO_CXX14_RETURN_TYPE_DEDUCTION
#define BHO_NO_CXX14_AGGREGATE_NSDMI
#define BHO_NO_CXX14_INITIALIZED_LAMBDA_CAPTURES
#define BHO_NO_CXX14_GENERIC_LAMBDAS
#define BHO_NO_CXX14_DIGIT_SEPARATORS
#define BHO_NO_CXX14_DECLTYPE_AUTO
#define BHO_NO_CXX14_CONSTEXPR
#define BHO_NO_CXX14_BINARY_LITERALS
#define BHO_NO_CXX17_STRUCTURED_BINDINGS
#define BHO_NO_CXX17_INLINE_VARIABLES
#define BHO_NO_CXX17_FOLD_EXPRESSIONS
#define BHO_NO_CXX17_IF_CONSTEXPR

// -------------------------------------

#if defined(__IBM_ATTRIBUTES)
#  define BHO_FORCEINLINE inline __attribute__ ((__always_inline__))
#  define BHO_NOINLINE __attribute__ ((__noinline__))
#  define BHO_MAY_ALIAS __attribute__((__may_alias__))
// No BHO_ALIGNMENT - explicit alignment support is broken (V2R1).
#endif

extern "builtin" long __builtin_expect(long, long);

#define BHO_LIKELY(x) __builtin_expect((x) && true, 1)
#define BHO_UNLIKELY(x) __builtin_expect((x) && true, 0)
