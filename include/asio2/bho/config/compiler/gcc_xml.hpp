//  (C) Copyright John Maddock 2006.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for most recent version.

//  GCC-XML C++ compiler setup:

#  if !defined(__GCCXML_GNUC__) || ((__GCCXML_GNUC__ <= 3) && (__GCCXML_GNUC_MINOR__ <= 3))
#     define BHO_NO_IS_ABSTRACT
#  endif

//
// Threading support: Turn this on unconditionally here (except for
// those platforms where we can know for sure). It will get turned off again
// later if no threading API is detected.
//
#if !defined(__MINGW32__) && !defined(_MSC_VER) && !defined(linux) && !defined(__linux) && !defined(__linux__)
# define BHO_HAS_THREADS
#endif

//
// gcc has "long long"
//
#define BHO_HAS_LONG_LONG

// C++0x features:
//
#  define BHO_NO_CXX11_CONSTEXPR
#  define BHO_NO_CXX11_NULLPTR
#  define BHO_NO_CXX11_TEMPLATE_ALIASES
#  define BHO_NO_CXX11_DECLTYPE
#  define BHO_NO_CXX11_DECLTYPE_N3276
#  define BHO_NO_CXX11_FUNCTION_TEMPLATE_DEFAULT_ARGS
#  define BHO_NO_CXX11_RVALUE_REFERENCES
#  define BHO_NO_CXX11_STATIC_ASSERT
#  define BHO_NO_CXX11_VARIADIC_TEMPLATES
#  define BHO_NO_CXX11_VARIADIC_MACROS
#  define BHO_NO_CXX11_AUTO_DECLARATIONS
#  define BHO_NO_CXX11_AUTO_MULTIDECLARATIONS
#  define BHO_NO_CXX11_CHAR16_T
#  define BHO_NO_CXX11_CHAR32_T
#  define BHO_NO_CXX11_DEFAULTED_FUNCTIONS
#  define BHO_NO_CXX11_DELETED_FUNCTIONS
#  define BHO_NO_CXX11_HDR_INITIALIZER_LIST
#  define BHO_NO_CXX11_SCOPED_ENUMS
#  define BHO_NO_SFINAE_EXPR
#  define BHO_NO_CXX11_SFINAE_EXPR
#  define BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS
#  define BHO_NO_CXX11_LAMBDAS
#  define BHO_NO_CXX11_LOCAL_CLASS_TEMPLATE_PARAMETERS
#  define BHO_NO_CXX11_RANGE_BASED_FOR
#  define BHO_NO_CXX11_RAW_LITERALS
#  define BHO_NO_CXX11_UNICODE_LITERALS
#  define BHO_NO_CXX11_NOEXCEPT
#  define BHO_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
#  define BHO_NO_CXX11_USER_DEFINED_LITERALS
#  define BHO_NO_CXX11_ALIGNAS
#  define BHO_NO_CXX11_ALIGNOF
#  define BHO_NO_CXX11_TRAILING_RESULT_TYPES
#  define BHO_NO_CXX11_INLINE_NAMESPACES
#  define BHO_NO_CXX11_REF_QUALIFIERS
#  define BHO_NO_CXX11_FINAL
#  define BHO_NO_CXX11_OVERRIDE
#  define BHO_NO_CXX11_THREAD_LOCAL
#  define BHO_NO_CXX11_UNRESTRICTED_UNION

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
#if !defined(__cpp_decltype_auto) || (__cpp_decltype_auto < 201304)
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

#define BHO_COMPILER "GCC-XML C++ version " __GCCXML__


