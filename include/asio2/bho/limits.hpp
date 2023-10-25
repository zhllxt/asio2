
//  (C) Copyright John maddock 1999. 
//  (C) David Abrahams 2002.  Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// use this header as a workaround for missing <limits>

//  See http://www.boost.org/libs/compatibility/index.html for documentation.

#ifndef BHO_LIMITS
#define BHO_LIMITS

#include <asio2/bho/config.hpp>

#ifdef BHO_NO_LIMITS
#  error "There is no std::numeric_limits suppport available."
#else
# include <limits>
#endif

#if (!defined(BOOST_LLT)) && (!defined(BOOST_ULLT)) \
      && ((defined(BHO_HAS_LONG_LONG) && defined(BHO_NO_LONG_LONG_NUMERIC_LIMITS)) \
      ||  (defined(BHO_HAS_MS_INT64 ) && defined(BHO_NO_MS_INT64_NUMERIC_LIMITS )))
// Add missing specializations for numeric_limits:
#ifdef BHO_HAS_MS_INT64
#  define BHO_LLT __int64
#  define BHO_ULLT unsigned __int64
#else
#  define BHO_LLT  ::bho::long_long_type
#  define BHO_ULLT  ::bho::ulong_long_type
#endif

#include <climits>  // for CHAR_BIT

namespace std
{
  template<>
  class numeric_limits<BHO_LLT> 
  {
   public:

      BHO_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef BHO_HAS_MS_INT64
      static BHO_LLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return 0x8000000000000000i64; }
      static BHO_LLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return 0x7FFFFFFFFFFFFFFFi64; }
#elif defined(LLONG_MAX)
      static BHO_LLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MIN; }
      static BHO_LLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return LLONG_MAX; }
#elif defined(LONGLONG_MAX)
      static BHO_LLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MIN; }
      static BHO_LLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return LONGLONG_MAX; }
#else
      static BHO_LLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return 1LL << (sizeof(BHO_LLT) * CHAR_BIT - 1); }
      static BHO_LLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return ~(min)(); }
#endif
      BHO_STATIC_CONSTANT(int, digits = sizeof(BHO_LLT) * CHAR_BIT -1);
      BHO_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (BHO_LLT) - 1) * 301L / 1000);
      BHO_STATIC_CONSTANT(bool, is_signed = true);
      BHO_STATIC_CONSTANT(bool, is_integer = true);
      BHO_STATIC_CONSTANT(bool, is_exact = true);
      BHO_STATIC_CONSTANT(int, radix = 2);
      static BHO_LLT epsilon() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_LLT round_error() BHO_NOEXCEPT_OR_NOTHROW { return 0; };

      BHO_STATIC_CONSTANT(int, min_exponent = 0);
      BHO_STATIC_CONSTANT(int, min_exponent10 = 0);
      BHO_STATIC_CONSTANT(int, max_exponent = 0);
      BHO_STATIC_CONSTANT(int, max_exponent10 = 0);

      BHO_STATIC_CONSTANT(bool, has_infinity = false);
      BHO_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      BHO_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      BHO_STATIC_CONSTANT(bool, has_denorm = false);
      BHO_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static BHO_LLT infinity() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_LLT quiet_NaN() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_LLT signaling_NaN() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_LLT denorm_min() BHO_NOEXCEPT_OR_NOTHROW { return 0; };

      BHO_STATIC_CONSTANT(bool, is_iec559 = false);
      BHO_STATIC_CONSTANT(bool, is_bounded = true);
      BHO_STATIC_CONSTANT(bool, is_modulo = true);

      BHO_STATIC_CONSTANT(bool, traps = false);
      BHO_STATIC_CONSTANT(bool, tinyness_before = false);
      BHO_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);
      
  };

  template<>
  class numeric_limits<BHO_ULLT> 
  {
   public:

      BHO_STATIC_CONSTANT(bool, is_specialized = true);
#ifdef BHO_HAS_MS_INT64
      static BHO_ULLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return 0ui64; }
      static BHO_ULLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return 0xFFFFFFFFFFFFFFFFui64; }
#elif defined(ULLONG_MAX) && defined(ULLONG_MIN)
      static BHO_ULLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MIN; }
      static BHO_ULLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return ULLONG_MAX; }
#elif defined(ULONGLONG_MAX) && defined(ULONGLONG_MIN)
      static BHO_ULLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MIN; }
      static BHO_ULLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return ULONGLONG_MAX; }
#else
      static BHO_ULLT min BHO_PREVENT_MACRO_SUBSTITUTION (){ return 0uLL; }
      static BHO_ULLT max BHO_PREVENT_MACRO_SUBSTITUTION (){ return ~0uLL; }
#endif
      BHO_STATIC_CONSTANT(int, digits = sizeof(BHO_LLT) * CHAR_BIT);
      BHO_STATIC_CONSTANT(int, digits10 = (CHAR_BIT * sizeof (BHO_LLT)) * 301L / 1000);
      BHO_STATIC_CONSTANT(bool, is_signed = false);
      BHO_STATIC_CONSTANT(bool, is_integer = true);
      BHO_STATIC_CONSTANT(bool, is_exact = true);
      BHO_STATIC_CONSTANT(int, radix = 2);
      static BHO_ULLT epsilon() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_ULLT round_error() BHO_NOEXCEPT_OR_NOTHROW { return 0; };

      BHO_STATIC_CONSTANT(int, min_exponent = 0);
      BHO_STATIC_CONSTANT(int, min_exponent10 = 0);
      BHO_STATIC_CONSTANT(int, max_exponent = 0);
      BHO_STATIC_CONSTANT(int, max_exponent10 = 0);

      BHO_STATIC_CONSTANT(bool, has_infinity = false);
      BHO_STATIC_CONSTANT(bool, has_quiet_NaN = false);
      BHO_STATIC_CONSTANT(bool, has_signaling_NaN = false);
      BHO_STATIC_CONSTANT(bool, has_denorm = false);
      BHO_STATIC_CONSTANT(bool, has_denorm_loss = false);
      static BHO_ULLT infinity() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_ULLT quiet_NaN() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_ULLT signaling_NaN() BHO_NOEXCEPT_OR_NOTHROW { return 0; };
      static BHO_ULLT denorm_min() BHO_NOEXCEPT_OR_NOTHROW { return 0; };

      BHO_STATIC_CONSTANT(bool, is_iec559 = false);
      BHO_STATIC_CONSTANT(bool, is_bounded = true);
      BHO_STATIC_CONSTANT(bool, is_modulo = true);

      BHO_STATIC_CONSTANT(bool, traps = false);
      BHO_STATIC_CONSTANT(bool, tinyness_before = false);
      BHO_STATIC_CONSTANT(float_round_style, round_style = round_toward_zero);
      
  };
}
#endif 

#endif

