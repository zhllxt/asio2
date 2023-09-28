// Three-state boolean logic library

// Copyright Douglas Gregor 2002-2004. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


// For more information, see http://www.boost.org
#ifndef BHO_LOGIC_TRIBOOL_HPP
#define BHO_LOGIC_TRIBOOL_HPP

#include <asio2/bho/logic/tribool_fwd.hpp>
#include <asio2/bho/config.hpp>
#include <asio2/bho/detail/workaround.hpp>

#ifdef BHO_HAS_PRAGMA_ONCE
#  pragma once
#endif

namespace bho { namespace logic {

/// INTERNAL ONLY
namespace detail {
/**
 * INTERNAL ONLY
 *
 * \brief A type used only to uniquely identify the 'indeterminate'
 * function/keyword.
 */
struct indeterminate_t
{
#if BHO_WORKAROUND(BHO_BORLANDC, < 0x0600)
  char dummy_; // BCB would use 8 bytes by default
#endif
};

} // end namespace detail

/**
 * INTERNAL ONLY
 * The type of the 'indeterminate' keyword. This has the same type as the
 * function 'indeterminate' so that we can recognize when the keyword is
 * used.
 */
typedef bool (*indeterminate_keyword_t)(tribool, detail::indeterminate_t);

/**
 * \brief Keyword and test function for the indeterminate tribool value
 *
 * The \c indeterminate function has a dual role. It's first role is
 * as a unary function that tells whether the tribool value is in the
 * "indeterminate" state. It's second role is as a keyword
 * representing the indeterminate (just like "true" and "false"
 * represent the true and false states). If you do not like the name
 * "indeterminate", and would prefer to use a different name, see the
 * macro \c BHO_TRIBOOL_THIRD_STATE.
 *
 * \returns <tt>x.value == tribool::indeterminate_value</tt>
 * \throws nothrow
 */
BHO_CONSTEXPR inline bool
indeterminate(tribool x,
              detail::indeterminate_t dummy = detail::indeterminate_t()) BHO_NOEXCEPT;

/**
 * \brief A 3-state boolean type.
 *
 * 3-state boolean values are either true, false, or
 * indeterminate.
 */
class tribool
{
#if defined( BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS )
private:
  /// INTERNAL ONLY
  struct dummy {
    void nonnull() {};
  };

  typedef void (dummy::*safe_bool)();
#endif

public:
  /**
   * Construct a new 3-state boolean value with the value 'false'.
   *
   * \throws nothrow
   */
  BHO_CONSTEXPR tribool() BHO_NOEXCEPT : value(false_value) {}

  /**
   * Construct a new 3-state boolean value with the given boolean
   * value, which may be \c true or \c false.
   *
   * \throws nothrow
   */
  BHO_CONSTEXPR tribool(bool initial_value) BHO_NOEXCEPT : value(initial_value? true_value : false_value) {}

  /**
   * Construct a new 3-state boolean value with an indeterminate value.
   *
   * \throws nothrow
   */
  BHO_CONSTEXPR tribool(indeterminate_keyword_t) BHO_NOEXCEPT : value(indeterminate_value) {}

  /**
   * Use a 3-state boolean in a boolean context. Will evaluate true in a
   * boolean context only when the 3-state boolean is definitely true.
   *
   * \returns true if the 3-state boolean is true, false otherwise
   * \throws nothrow
   */
#if !defined( BHO_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS )

  BHO_CONSTEXPR explicit operator bool () const BHO_NOEXCEPT
  {
    return value == true_value;
  }

#else

  BHO_CONSTEXPR operator safe_bool() const BHO_NOEXCEPT
  {
    return value == true_value? &dummy::nonnull : 0;
  }

#endif

  /**
   * The actual stored value in this 3-state boolean, which may be false, true,
   * or indeterminate.
   */
  enum value_t { false_value, true_value, indeterminate_value } value;
};

// Check if the given tribool has an indeterminate value. Also doubles as a
// keyword for the 'indeterminate' value
BHO_CONSTEXPR inline bool indeterminate(tribool x, detail::indeterminate_t) BHO_NOEXCEPT
{
  return x.value == tribool::indeterminate_value;
}

/** @defgroup logical Logical operations
 */
//@{
/**
 * \brief Computes the logical negation of a tribool
 *
 * \returns the logical negation of the tribool, according to the
 * table:
 *  <table border=1>
 *    <tr>
 *      <th><center><code>!</code></center></th>
 *      <th/>
 *    </tr>
 *    <tr>
 *      <th><center>false</center></th>
 *      <td><center>true</center></td>
 *    </tr>
 *    <tr>
 *      <th><center>true</center></th>
 *      <td><center>false</center></td>
 *    </tr>
 *    <tr>
 *      <th><center>indeterminate</center></th>
 *      <td><center>indeterminate</center></td>
 *    </tr>
 *  </table>
 * \throws nothrow
 */
BHO_CONSTEXPR inline tribool operator!(tribool x) BHO_NOEXCEPT
{
  return x.value == tribool::false_value? tribool(true)
        :x.value == tribool::true_value? tribool(false)
        :tribool(indeterminate);
}

/**
 * \brief Computes the logical conjunction of two tribools
 *
 * \returns the result of logically ANDing the two tribool values,
 * according to the following table:
 *       <table border=1>
 *           <tr>
 *             <th><center><code>&amp;&amp;</code></center></th>
 *             <th><center>false</center></th>
 *             <th><center>true</center></th>
 *             <th><center>indeterminate</center></th>
 *           </tr>
 *           <tr>
 *             <th><center>false</center></th>
 *             <td><center>false</center></td>
 *             <td><center>false</center></td>
 *             <td><center>false</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>true</center></th>
 *             <td><center>false</center></td>
 *             <td><center>true</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>indeterminate</center></th>
 *             <td><center>false</center></td>
 *             <td><center>indeterminate</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *       </table>
 * \throws nothrow
 */
BHO_CONSTEXPR inline tribool operator&&(tribool x, tribool y) BHO_NOEXCEPT
{
  return (static_cast<bool>(!x) || static_cast<bool>(!y))
    ? tribool(false)
    : ((static_cast<bool>(x) && static_cast<bool>(y)) ? tribool(true) : indeterminate)
  ;
}

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator&&(tribool x, bool y) BHO_NOEXCEPT
{ return y? x : tribool(false); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator&&(bool x, tribool y) BHO_NOEXCEPT
{ return x? y : tribool(false); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator&&(indeterminate_keyword_t, tribool x) BHO_NOEXCEPT
{ return !x? tribool(false) : tribool(indeterminate); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator&&(tribool x, indeterminate_keyword_t) BHO_NOEXCEPT
{ return !x? tribool(false) : tribool(indeterminate); }

/**
 * \brief Computes the logical disjunction of two tribools
 *
 * \returns the result of logically ORing the two tribool values,
 * according to the following table:
 *       <table border=1>
 *           <tr>
 *             <th><center><code>||</code></center></th>
 *             <th><center>false</center></th>
 *             <th><center>true</center></th>
 *             <th><center>indeterminate</center></th>
 *           </tr>
 *           <tr>
 *             <th><center>false</center></th>
 *             <td><center>false</center></td>
 *             <td><center>true</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>true</center></th>
 *             <td><center>true</center></td>
 *             <td><center>true</center></td>
 *             <td><center>true</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>indeterminate</center></th>
 *             <td><center>indeterminate</center></td>
 *             <td><center>true</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *       </table>
 *  \throws nothrow
 */
BHO_CONSTEXPR inline tribool operator||(tribool x, tribool y) BHO_NOEXCEPT
{
  return (static_cast<bool>(!x) && static_cast<bool>(!y))
    ? tribool(false)
    : ((static_cast<bool>(x) || static_cast<bool>(y)) ? tribool(true) : tribool(indeterminate))
  ;
}

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator||(tribool x, bool y) BHO_NOEXCEPT
{ return y? tribool(true) : x; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator||(bool x, tribool y) BHO_NOEXCEPT
{ return x? tribool(true) : y; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator||(indeterminate_keyword_t, tribool x) BHO_NOEXCEPT
{ return x? tribool(true) : tribool(indeterminate); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator||(tribool x, indeterminate_keyword_t) BHO_NOEXCEPT
{ return x? tribool(true) : tribool(indeterminate); }
//@}

/**
 * \brief Compare tribools for equality
 *
 * \returns the result of comparing two tribool values, according to
 * the following table:
 *       <table border=1>
 *          <tr>
 *            <th><center><code>==</code></center></th>
 *            <th><center>false</center></th>
 *            <th><center>true</center></th>
 *            <th><center>indeterminate</center></th>
 *          </tr>
 *          <tr>
 *            <th><center>false</center></th>
 *            <td><center>true</center></td>
 *            <td><center>false</center></td>
 *            <td><center>indeterminate</center></td>
 *          </tr>
 *          <tr>
 *            <th><center>true</center></th>
 *            <td><center>false</center></td>
 *            <td><center>true</center></td>
 *            <td><center>indeterminate</center></td>
 *          </tr>
 *          <tr>
 *            <th><center>indeterminate</center></th>
 *            <td><center>indeterminate</center></td>
 *            <td><center>indeterminate</center></td>
 *            <td><center>indeterminate</center></td>
 *          </tr>
 *      </table>
 * \throws nothrow
 */
BHO_CONSTEXPR inline tribool operator==(tribool x, tribool y) BHO_NOEXCEPT
{
  return (indeterminate(x) || indeterminate(y))
    ? indeterminate
    : ((x && y) || (!x && !y))
  ;
}

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator==(tribool x, bool y) BHO_NOEXCEPT { return x == tribool(y); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator==(bool x, tribool y) BHO_NOEXCEPT { return tribool(x) == y; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator==(indeterminate_keyword_t, tribool x) BHO_NOEXCEPT
{ return tribool(indeterminate) == x; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator==(tribool x, indeterminate_keyword_t) BHO_NOEXCEPT
{ return tribool(indeterminate) == x; }

/**
 * \brief Compare tribools for inequality
 *
 * \returns the result of comparing two tribool values for inequality,
 * according to the following table:
 *       <table border=1>
 *           <tr>
 *             <th><center><code>!=</code></center></th>
 *             <th><center>false</center></th>
 *             <th><center>true</center></th>
 *             <th><center>indeterminate</center></th>
 *           </tr>
 *           <tr>
 *             <th><center>false</center></th>
 *             <td><center>false</center></td>
 *             <td><center>true</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>true</center></th>
 *             <td><center>true</center></td>
 *             <td><center>false</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *           <tr>
 *             <th><center>indeterminate</center></th>
 *             <td><center>indeterminate</center></td>
 *             <td><center>indeterminate</center></td>
 *             <td><center>indeterminate</center></td>
 *           </tr>
 *       </table>
 * \throws nothrow
 */
BHO_CONSTEXPR inline tribool operator!=(tribool x, tribool y) BHO_NOEXCEPT
{
  return (indeterminate(x) || indeterminate(y))
    ? indeterminate
    : !((x && y) || (!x && !y))
  ;
}

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator!=(tribool x, bool y) BHO_NOEXCEPT { return x != tribool(y); }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator!=(bool x, tribool y) BHO_NOEXCEPT { return tribool(x) != y; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator!=(indeterminate_keyword_t, tribool x) BHO_NOEXCEPT
{ return tribool(indeterminate) != x; }

/**
 * \overload
 */
BHO_CONSTEXPR inline tribool operator!=(tribool x, indeterminate_keyword_t) BHO_NOEXCEPT
{ return x != tribool(indeterminate); }

} } // end namespace bho::logic

// Pull tribool and indeterminate into namespace "bho"
namespace bho {
  using logic::tribool;
  using logic::indeterminate;
}

/**
 * \brief Declare a new name for the third state of a tribool
 *
 * Use this macro to declare a new name for the third state of a
 * tribool. This state can have any number of new names (in addition
 * to \c indeterminate), all of which will be equivalent. The new name will be
 * placed in the namespace in which the macro is expanded.
 *
 * Example:
 *   BHO_TRIBOOL_THIRD_STATE(true_or_false)
 *
 *   tribool x(true_or_false);
 *   // potentially set x
 *   if (true_or_false(x)) {
 *     // don't know what x is
 *   }
 */
#define BHO_TRIBOOL_THIRD_STATE(Name)                                 \
inline bool                                                             \
Name(bho::logic::tribool x,                                           \
     bho::logic::detail::indeterminate_t =                            \
       bho::logic::detail::indeterminate_t())                         \
{ return x.value == bho::logic::tribool::indeterminate_value; }

#endif // BHO_LOGIC_TRIBOOL_HPP

