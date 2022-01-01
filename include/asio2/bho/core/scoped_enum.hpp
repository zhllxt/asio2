//  scoped_enum.hpp  ---------------------------------------------------------//

//  Copyright Beman Dawes, 2009
//  Copyright (C) 2011-2012 Vicente J. Botet Escriba
//  Copyright (C) 2012 Anthony Williams

//  Distributed under the Boost Software License, Version 1.0.
//  See http://www.boost.org/LICENSE_1_0.txt

#ifndef BHO_CORE_SCOPED_ENUM_HPP
#define BHO_CORE_SCOPED_ENUM_HPP

#include <asio2/bho/config.hpp>

#ifdef BHO_HAS_PRAGMA_ONCE
#pragma once
#endif

namespace bho
{

#ifdef BHO_NO_CXX11_SCOPED_ENUMS

  /**
   * Meta-function to get the native enum type associated to an enum class or its emulation.
   */
  template <typename EnumType>
  struct native_type
  {
    /**
     * The member typedef type names the native enum type associated to the scoped enum,
     * which is it self if the compiler supports scoped enums or EnumType::enum_type if it is an emulated scoped enum.
     */
    typedef typename EnumType::enum_type type;
  };

  /**
   * Casts a scoped enum to its underlying type.
   *
   * This function is useful when working with scoped enum classes, which doens't implicitly convert to the underlying type.
   * @param v A scoped enum.
   * @returns The underlying type.
   * @throws No-throws.
   */
  template <typename UnderlyingType, typename EnumType>
  inline
  BHO_CONSTEXPR UnderlyingType underlying_cast(EnumType v) BHO_NOEXCEPT
  {
    return v.get_underlying_value_();
  }

  /**
   * Casts a scoped enum to its native enum type.
   *
   * This function is useful to make programs portable when the scoped enum emulation can not be use where native enums can.
   *
   * EnumType the scoped enum type
   *
   * @param v A scoped enum.
   * @returns The native enum value.
   * @throws No-throws.
   */
  template <typename EnumType>
  inline
  BHO_CONSTEXPR typename EnumType::enum_type native_value(EnumType e) BHO_NOEXCEPT
  {
    return e.get_native_value_();
  }

#else  // BHO_NO_CXX11_SCOPED_ENUMS

  template <typename EnumType>
  struct native_type
  {
    typedef EnumType type;
  };

  template <typename UnderlyingType, typename EnumType>
  inline
  BHO_CONSTEXPR UnderlyingType underlying_cast(EnumType v) BHO_NOEXCEPT
  {
    return static_cast<UnderlyingType>(v);
  }

  template <typename EnumType>
  inline
  BHO_CONSTEXPR EnumType native_value(EnumType e) BHO_NOEXCEPT
  {
    return e;
  }

#endif // BHO_NO_CXX11_SCOPED_ENUMS
}


#ifdef BHO_NO_CXX11_SCOPED_ENUMS

#ifndef BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS

#define BHO_SCOPED_ENUM_UT_DECLARE_CONVERSION_OPERATOR \
     explicit BHO_CONSTEXPR operator underlying_type() const BHO_NOEXCEPT { return get_underlying_value_(); }

#else

#define BHO_SCOPED_ENUM_UT_DECLARE_CONVERSION_OPERATOR

#endif

/**
 * Start a declaration of a scoped enum.
 *
 * @param EnumType The new scoped enum.
 * @param UnderlyingType The underlying type.
 */
#define BHO_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType, UnderlyingType)    \
    struct EnumType {                                                   \
        typedef void is_bho_scoped_enum_tag;                          \
        typedef UnderlyingType underlying_type;                         \
        EnumType() BHO_NOEXCEPT {}                                    \
        explicit BHO_CONSTEXPR EnumType(underlying_type v) BHO_NOEXCEPT : v_(v) {}                 \
        BHO_CONSTEXPR underlying_type get_underlying_value_() const BHO_NOEXCEPT { return v_; }    \
        BHO_SCOPED_ENUM_UT_DECLARE_CONVERSION_OPERATOR                \
    private:                                                            \
        underlying_type v_;                                             \
        typedef EnumType self_type;                                     \
    public:                                                             \
        enum enum_type

#define BHO_SCOPED_ENUM_DECLARE_END2() \
        BHO_CONSTEXPR enum_type get_native_value_() const BHO_NOEXCEPT { return enum_type(v_); } \
        friend BHO_CONSTEXPR bool operator ==(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)==enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator ==(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)==rhs; } \
        friend BHO_CONSTEXPR bool operator ==(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs==enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator !=(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)!=enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator !=(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)!=rhs; } \
        friend BHO_CONSTEXPR bool operator !=(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs!=enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator <(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)<enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator <(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)<rhs; } \
        friend BHO_CONSTEXPR bool operator <(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs<enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator <=(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)<=enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator <=(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)<=rhs; } \
        friend BHO_CONSTEXPR bool operator <=(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs<=enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator >(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)>enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator >(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)>rhs; } \
        friend BHO_CONSTEXPR bool operator >(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs>enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator >=(self_type lhs, self_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)>=enum_type(rhs.v_); } \
        friend BHO_CONSTEXPR bool operator >=(self_type lhs, enum_type rhs) BHO_NOEXCEPT { return enum_type(lhs.v_)>=rhs; } \
        friend BHO_CONSTEXPR bool operator >=(enum_type lhs, self_type rhs) BHO_NOEXCEPT { return lhs>=enum_type(rhs.v_); } \
    };

#define BHO_SCOPED_ENUM_DECLARE_END(EnumType) \
    ; \
    BHO_CONSTEXPR EnumType(enum_type v) BHO_NOEXCEPT : v_(v) {}                 \
    BHO_SCOPED_ENUM_DECLARE_END2()

/**
 * Starts a declaration of a scoped enum with the default int underlying type.
 *
 * @param EnumType The new scoped enum.
 */
#define BHO_SCOPED_ENUM_DECLARE_BEGIN(EnumType) \
  BHO_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType,int)

/**
 * Name of the native enum type.
 *
 * @param EnumType The new scoped enum.
 */
#define BHO_SCOPED_ENUM_NATIVE(EnumType) EnumType::enum_type
/**
 * Forward declares an scoped enum.
 *
 * @param EnumType The scoped enum.
 */
#define BHO_SCOPED_ENUM_FORWARD_DECLARE(EnumType) struct EnumType

#else  // BHO_NO_CXX11_SCOPED_ENUMS

#define BHO_SCOPED_ENUM_UT_DECLARE_BEGIN(EnumType,UnderlyingType) enum class EnumType : UnderlyingType
#define BHO_SCOPED_ENUM_DECLARE_BEGIN(EnumType) enum class EnumType
#define BHO_SCOPED_ENUM_DECLARE_END2()
#define BHO_SCOPED_ENUM_DECLARE_END(EnumType) ;

#define BHO_SCOPED_ENUM_NATIVE(EnumType) EnumType
#define BHO_SCOPED_ENUM_FORWARD_DECLARE(EnumType) enum class EnumType

#endif  // BHO_NO_CXX11_SCOPED_ENUMS

// Deprecated macros
#define BHO_SCOPED_ENUM_START(name) BHO_SCOPED_ENUM_DECLARE_BEGIN(name)
#define BHO_SCOPED_ENUM_END BHO_SCOPED_ENUM_DECLARE_END2()
#define BHO_SCOPED_ENUM(name) BHO_SCOPED_ENUM_NATIVE(name)

#endif  // BHO_CORE_SCOPED_ENUM_HPP
