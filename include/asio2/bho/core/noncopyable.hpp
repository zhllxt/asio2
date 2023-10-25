//  Boost noncopyable.hpp header file  --------------------------------------//

//  (C) Copyright Beman Dawes 1999-2003. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/utility for documentation.

#ifndef BHO_CORE_NONCOPYABLE_HPP
#define BHO_CORE_NONCOPYABLE_HPP

#include <asio2/bho/config.hpp>

namespace bho {

//  Private copy constructor and copy assignment ensure classes derived from
//  class noncopyable cannot be copied.

//  Contributed by Dave Abrahams

namespace noncopyable_  // protection from unintended ADL
{
#ifndef BHO_NONCOPYABLE_BASE_TOKEN_DEFINED
#define BHO_NONCOPYABLE_BASE_TOKEN_DEFINED

// noncopyable derives from base_token to enable Type Traits to detect
// whether a type derives from noncopyable without needing the definition
// of noncopyable itself.
//
// The definition of base_token is macro-guarded so that Type Traits can
// define it locally without including this header, to avoid a dependency
// on Core.

  struct base_token {};

#endif // #ifndef BHO_NONCOPYABLE_BASE_TOKEN_DEFINED

  class noncopyable: base_token
  {
  protected:
#if !defined(BHO_NO_CXX11_DEFAULTED_FUNCTIONS) && !defined(BHO_NO_CXX11_NON_PUBLIC_DEFAULTED_FUNCTIONS)
      BHO_CONSTEXPR noncopyable() = default;
      ~noncopyable() = default;
#else
      noncopyable() {}
      ~noncopyable() {}
#endif
#if !defined(BHO_NO_CXX11_DELETED_FUNCTIONS)
      noncopyable( const noncopyable& ) = delete;
      noncopyable& operator=( const noncopyable& ) = delete;
#else
  private:  // emphasize the following members are private
      noncopyable( const noncopyable& );
      noncopyable& operator=( const noncopyable& );
#endif
  };
}

typedef noncopyable_::noncopyable noncopyable;

} // namespace bho

#endif  // BHO_CORE_NONCOPYABLE_HPP
