//  (C) Copyright John Maddock 2001.
//  (C) Copyright Jens Maurer 2001 - 2003.
//  (C) Copyright Peter Dimov 2002.
//  (C) Copyright Aleksey Gurtovoy 2002 - 2003.
//  (C) Copyright David Abrahams 2002.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  Sun C++ compiler setup:

#    if __SUNPRO_CC <= 0x500
#      define BHO_NO_MEMBER_TEMPLATES
#      define BHO_NO_FUNCTION_TEMPLATE_ORDERING
#    endif

#    if (__SUNPRO_CC <= 0x520)
       //
       // Sunpro 5.2 and earler:
       //
       // although sunpro 5.2 supports the syntax for
       // inline initialization it often gets the value
       // wrong, especially where the value is computed
       // from other constants (J Maddock 6th May 2001)
#      define BHO_NO_INCLASS_MEMBER_INITIALIZATION

       // Although sunpro 5.2 supports the syntax for
       // partial specialization, it often seems to
       // bind to the wrong specialization.  Better
       // to disable it until suppport becomes more stable
       // (J Maddock 6th May 2001).
#      define BHO_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#    endif

#    if (__SUNPRO_CC <= 0x530)
       // Requesting debug info (-g) with BHO.Python results
       // in an internal compiler error for "static const"
       // initialized in-class.
       //    >> Assertion:   (../links/dbg_cstabs.cc, line 611)
       //         while processing ../test.cpp at line 0.
       // (Jens Maurer according to Gottfried Ganssauge 04 Mar 2002)
#      define BHO_NO_INCLASS_MEMBER_INITIALIZATION

       // SunPro 5.3 has better support for partial specialization,
       // but breaks when compiling std::less<shared_ptr<T> >
       // (Jens Maurer 4 Nov 2001).

       // std::less specialization fixed as reported by George
       // Heintzelman; partial specialization re-enabled
       // (Peter Dimov 17 Jan 2002)

//#      define BHO_NO_TEMPLATE_PARTIAL_SPECIALIZATION

       // integral constant expressions with 64 bit numbers fail
#      define BHO_NO_INTEGRAL_INT64_T
#    endif

#    if (__SUNPRO_CC < 0x570)
#      define BHO_NO_TEMPLATE_TEMPLATES
       // see http://lists.boost.org/MailArchives/bho/msg47184.php
       // and http://lists.boost.org/MailArchives/bho/msg47220.php
#      define BHO_NO_INCLASS_MEMBER_INITIALIZATION
#      define BHO_NO_SFINAE
#      define BHO_NO_ARRAY_TYPE_SPECIALIZATIONS
#    endif
#    if (__SUNPRO_CC <= 0x580)
#      define BHO_NO_IS_ABSTRACT
#    endif

#    if (__SUNPRO_CC <= 0x5100)
       // Sun 5.10 may not correctly value-initialize objects of
       // some user defined types, as was reported in April 2010
       // (CR 6947016), and confirmed by Steve Clamage.
       // (Niels Dekker, LKEB, May 2010).
#      define BHO_NO_COMPLETE_VALUE_INITIALIZATION
#    endif

//
// Dynamic shared object (DSO) and dynamic-link library (DLL) support
//
#if __SUNPRO_CC > 0x500
#  define BHO_SYMBOL_EXPORT __global
#  define BHO_SYMBOL_IMPORT __global
#  define BHO_SYMBOL_VISIBLE __global
#endif

// Deprecated symbol markup
// Oracle Studio 12.4 supports deprecated attribute with a message; this is the first release that supports the attribute.
#if (__SUNPRO_CC >= 0x5130)
#define BHO_DEPRECATED(msg) __attribute__((deprecated(msg)))
#endif

#if (__SUNPRO_CC < 0x5130)
// C++03 features in 12.4:
#define BHO_NO_TWO_PHASE_NAME_LOOKUP
#define BHO_NO_SFINAE_EXPR
#define BHO_NO_ADL_BARRIER
#define BHO_NO_CXX11_VARIADIC_MACROS
#endif

#if (__SUNPRO_CC < 0x5130) || (__cplusplus < 201100)
// C++11 only featuires in 12.4:
#define BHO_NO_CXX11_AUTO_DECLARATIONS
#define BHO_NO_CXX11_AUTO_MULTIDECLARATIONS
#define BHO_NO_CXX11_CHAR16_T
#define BHO_NO_CXX11_CHAR32_T
#define BHO_NO_CXX11_CONSTEXPR
#define BHO_NO_CXX11_DECLTYPE
#define BHO_NO_CXX11_DEFAULTED_FUNCTIONS
#define BHO_NO_CXX11_DELETED_FUNCTIONS
#define BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#define BHO_NO_CXX11_EXTERN_TEMPLATE
#define BHO_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#define BHO_NO_CXX11_LAMBDAS
#define BHO_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#define BHO_NO_CXX11_NOEXCEPT
#define BHO_NO_CXX11_NULLPTR
#define BHO_NO_CXX11_RANGE_BASED_FOR
#define BHO_NO_CXX11_RAW_LITERALS
#define BHO_NO_CXX11_RVALUE_REFERENCES
#define BHO_NO_CXX11_SCOPED_ENUMS
#define BHO_NO_CXX11_STATIC_ASSERT
#define BHO_NO_CXX11_TEMPLATE_ALIASES
#define BHO_NO_CXX11_UNICODE_LITERALS
#define BHO_NO_CXX11_ALIGNAS
#define BHO_NO_CXX11_ALIGNOF
#define BHO_NO_CXX11_TRAILING_RESULT_TYPES
#define BHO_NO_CXX11_INLINE_NAMESPACES
#define BHO_NO_CXX11_FINAL
#define BHO_NO_CXX11_OVERRIDE
#define BHO_NO_CXX11_UNRESTRICTED_UNION
#endif

#if (__SUNPRO_CC < 0x5140) || (__cplusplus < 201103)
#define BHO_NO_CXX11_VARIADIC_TEMPLATES
#define BHO_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#define BHO_NO_CXX11_FIXED_LENGTH_VARIADIC_TEMPLATE_EXPANSION_PACKS
#define BHO_NO_CXX11_DECLTYPE_N3276
#define BHO_NO_CXX11_USER_DEFINED_LITERALS
#define BHO_NO_CXX11_REF_QUALIFIERS
#define BHO_NO_CXX11_THREAD_LOCAL
#endif

#define BHO_NO_COMPLETE_VALUE_INITIALIZATION
//
// C++0x features
//
#  define BHO_HAS_LONG_LONG

#define BHO_NO_CXX11_SFINAE_EXPR

// C++ 14:
#if !defined(__cpp_aggregate_nsdmi) || (__cpp_aggregate_nsdmi < 201304)
#  define BHO_NO_CXX14_AGGREGATE_NSDMI
#endif
#if !defined(__cpp_binary_literals) || (__cpp_binary_literals < 201304)
#  define BHO_NO_CXX14_BINARY_LITERALS
#endif
#if !defined(__cpp_constexpr) || (__cpp_constexpr < 201304)
#  define BHO_NO_CXX14_CONSTEXPR
#endif
#if !defined(__cpp_decltype_auto) || (__cpp_decltype_auto < 201304) || (__cplusplus < 201402L)
#  define BHO_NO_CXX14_DECLTYPE_AUTO
#endif
#if (__cplusplus < 201304) // There's no SD6 check for this....
#  define BHO_NO_CXX14_DIGIT_SEPARATORS
#endif
#if !defined(__cpp_generic_lambdas) || (__cpp_generic_lambdas < 201304)
#  define BHO_NO_CXX14_GENERIC_LAMBDAS
#endif
#if !defined(__cpp_init_captures) || (__cpp_init_captures < 201304)
#  define BHO_NO_CXX14_INITIALIZED_LAMBDA_CAPTURES
#endif
#if !defined(__cpp_return_type_deduction) || (__cpp_return_type_deduction < 201304)
#  define BHO_NO_CXX14_RETURN_TYPE_DEDUCTION
#endif
#if !defined(__cpp_variable_templates) || (__cpp_variable_templates < 201304)
#  define BHO_NO_CXX14_VARIABLE_TEMPLATES
#endif

// C++17
#if !defined(__cpp_structured_bindings) || (__cpp_structured_bindings < 201606)
#  define BHO_NO_CXX17_STRUCTURED_BINDINGS
#endif
#if !defined(__cpp_inline_variables) || (__cpp_inline_variables < 201606)
#  define BHO_NO_CXX17_INLINE_VARIABLES
#endif
#if !defined(__cpp_fold_expressions) || (__cpp_fold_expressions < 201603)
#  define BHO_NO_CXX17_FOLD_EXPRESSIONS
#endif
#if !defined(__cpp_if_constexpr) || (__cpp_if_constexpr < 201606)
#  define BHO_NO_CXX17_IF_CONSTEXPR
#endif

// Turn on threading support for Solaris 12.
// Ticket #11972
#if (__SUNPRO_CC >= 0x5140) && defined(__SunOS_5_12) && !defined(BHO_HAS_THREADS)
# define BHO_HAS_THREADS
#endif

//
// Version
//

#define BHO_COMPILER "Sun compiler version " BHO_STRINGIZE(__SUNPRO_CC)

//
// versions check:
// we don't support sunpro prior to version 4:
#if __SUNPRO_CC < 0x400
#error "Compiler not supported or configured - please reconfigure"
#endif
//
// last known and checked version:
#if (__SUNPRO_CC > 0x5150)
#  if defined(BHO_ASSERT_CONFIG)
#     error "BHO.Config is older than your compiler - please check for an updated Boost release."
#  endif
#endif
