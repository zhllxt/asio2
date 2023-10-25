#ifndef BHO_DESCRIBE_MODIFIER_DESCRIPTION_HPP_INCLUDED
#define BHO_DESCRIBE_MODIFIER_DESCRIPTION_HPP_INCLUDED

// Copyright 2020, 2022 Peter Dimov
// Distributed under the Boost Software License, Version 1.0.
// https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/describe/modifiers.hpp>
#include <asio2/bho/describe/enum.hpp>

namespace bho
{
namespace describe
{

BHO_DESCRIBE_ENUM(modifiers,
    mod_public,
    mod_protected,
    mod_private,
    mod_virtual,
    mod_static,
    mod_function,
    mod_any_member,
    mod_inherited,
    mod_hidden)

} // namespace describe
} // namespace bho

#endif // #ifndef BHO_DESCRIBE_MODIFIER_DESCRIPTION_HPP_INCLUDED
