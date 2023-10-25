#ifndef BHO_DESCRIBE_MODIFIERS_HPP_INCLUDED
#define BHO_DESCRIBE_MODIFIERS_HPP_INCLUDED

// Copyright 2020 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/detail/config.hpp>

namespace bho
{
namespace describe
{

enum modifiers
{
    mod_public = 1,
    mod_protected = 2,
    mod_private = 4,
    mod_virtual = 8,
    mod_static = 16,
    mod_function = 32,
    mod_any_member = 64,
    mod_inherited = 128,
    mod_hidden = 256
};

BHO_DESCRIBE_CONSTEXPR_OR_CONST modifiers mod_any_access = static_cast<modifiers>( mod_public | mod_protected | mod_private );

} // namespace describe
} // namespace bho

#endif // #ifndef BHO_DESCRIBE_MODIFIERS_HPP_INCLUDED
