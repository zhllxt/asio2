#ifndef BHO_DESCRIBE_DETAIL_CONFIG_HPP_INCLUDED
#define BHO_DESCRIBE_DETAIL_CONFIG_HPP_INCLUDED

// Copyright 2021 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#if __cplusplus >= 201402L

# define BHO_DESCRIBE_CXX14
# define BHO_DESCRIBE_CXX11

#elif defined(_MSC_VER) && _MSC_VER >= 1900

# define BHO_DESCRIBE_CXX14
# define BHO_DESCRIBE_CXX11

#elif __cplusplus >= 201103L

# define BHO_DESCRIBE_CXX11

# if defined(__GNUC__) && __GNUC__ == 4 && __GNUC_MINOR__ == 7
#  undef BHO_DESCRIBE_CXX11
# endif

#endif

#if defined(BHO_DESCRIBE_CXX11)
# define BHO_DESCRIBE_CONSTEXPR_OR_CONST constexpr
#else
# define BHO_DESCRIBE_CONSTEXPR_OR_CONST const
#endif

#if defined(__clang__)
# define BHO_DESCRIBE_MAYBE_UNUSED __attribute__((unused))
#else
# define BHO_DESCRIBE_MAYBE_UNUSED
#endif

#endif // #ifndef BHO_DESCRIBE_DETAIL_CONFIG_HPP_INCLUDED
