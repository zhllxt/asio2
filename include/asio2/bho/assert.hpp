//
//  bho/assert.hpp - BHO_ASSERT(expr)
//                     BHO_ASSERT_MSG(expr, msg)
//                     BHO_VERIFY(expr)
//                     BHO_VERIFY_MSG(expr, msg)
//                     BHO_ASSERT_IS_VOID
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2007, 2014 Peter Dimov
//  Copyright (c) Beman Dawes 2011
//  Copyright (c) 2015 Ion Gaztanaga
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  Note: There are no include guards. This is intentional.
//
//  See http://www.boost.org/libs/assert/assert.html for documentation.
//

//
// Stop inspect complaining about use of 'assert':
//
// bhoinspect:naassert_macro
//

//
// BHO_ASSERT, BHO_ASSERT_MSG, BHO_ASSERT_IS_VOID
//

#undef BHO_ASSERT
#undef BHO_ASSERT_MSG
#undef BHO_ASSERT_IS_VOID

#if defined(BHO_DISABLE_ASSERTS) || ( defined(BHO_ENABLE_ASSERT_DEBUG_HANDLER) && defined(NDEBUG) )

# define BHO_ASSERT(expr) ((void)0)
# define BHO_ASSERT_MSG(expr, msg) ((void)0)
# define BHO_ASSERT_IS_VOID

#elif defined(BHO_ENABLE_ASSERT_HANDLER) || ( defined(BHO_ENABLE_ASSERT_DEBUG_HANDLER) && !defined(NDEBUG) )

#include <asio2/bho/config.hpp> // for BHO_LIKELY
#include <asio2/bho/current_function.hpp>

namespace bho
{
    void assertion_failed(char const * expr, char const * function, char const * file, long line); // user defined
    void assertion_failed_msg(char const * expr, char const * msg, char const * function, char const * file, long line); // user defined
} // namespace bho

#define BHO_ASSERT(expr) (BHO_LIKELY(!!(expr))? ((void)0): ::bho::assertion_failed(#expr, BHO_CURRENT_FUNCTION, __FILE__, __LINE__))
#define BHO_ASSERT_MSG(expr, msg) (BHO_LIKELY(!!(expr))? ((void)0): ::bho::assertion_failed_msg(#expr, msg, BHO_CURRENT_FUNCTION, __FILE__, __LINE__))

#else

# include <assert.h> // .h to support old libraries w/o <cassert> - effect is the same

# define BHO_ASSERT(expr) assert(expr)
# define BHO_ASSERT_MSG(expr, msg) assert((expr)&&(msg))
#if defined(NDEBUG)
# define BHO_ASSERT_IS_VOID
#endif

#endif

//
// BHO_VERIFY, BHO_VERIFY_MSG
//

#undef BHO_VERIFY
#undef BHO_VERIFY_MSG

#if defined(BHO_DISABLE_ASSERTS) || ( !defined(BHO_ENABLE_ASSERT_HANDLER) && defined(NDEBUG) )

# define BHO_VERIFY(expr) ((void)(expr))
# define BHO_VERIFY_MSG(expr, msg) ((void)(expr))

#else

# define BHO_VERIFY(expr) BHO_ASSERT(expr)
# define BHO_VERIFY_MSG(expr, msg) BHO_ASSERT_MSG(expr,msg)

#endif
