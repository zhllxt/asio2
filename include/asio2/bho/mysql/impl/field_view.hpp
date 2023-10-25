//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_FIELD_VIEW_HPP
#define BHO_MYSQL_IMPL_FIELD_VIEW_HPP

#pragma once

#include <asio2/bho/mysql/bad_field_access.hpp>
#include <asio2/bho/mysql/field_view.hpp>

#include <asio2/bho/assert.hpp>
#include <asio2/bho/throw_exception.hpp>

#include <cstring>
#include <limits>

namespace bho {
namespace mysql {
namespace detail {

inline bool blobs_equal(blob_view b1, blob_view b2)
{
    if (b1.size() != b2.size())
        return false;
    return b1.empty() || std::memcmp(b1.data(), b2.data(), b2.size()) == 0;
}

}  // namespace detail
}  // namespace mysql
}  // namespace bho

BHO_CXX14_CONSTEXPR inline bho::mysql::field_kind bho::mysql::field_view::kind() const noexcept
{
    switch (impl_.ikind)
    {
    case internal_kind::null: return field_kind::null;
    case internal_kind::int64: return field_kind::int64;
    case internal_kind::uint64: return field_kind::uint64;
    case internal_kind::string: return field_kind::string;
    case internal_kind::blob: return field_kind::blob;
    case internal_kind::float_: return field_kind::float_;
    case internal_kind::double_: return field_kind::double_;
    case internal_kind::date: return field_kind::date;
    case internal_kind::datetime: return field_kind::datetime;
    case internal_kind::time: return field_kind::time;
    case internal_kind::field_ptr: return impl_.repr.field_ptr->kind();
    // sv_offset values must be converted via offset_to_string_view before calling any other fn
    default: return field_kind::null;
    }
}

BHO_CXX14_CONSTEXPR std::int64_t bho::mysql::field_view::as_int64() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<std::int64_t>();
    check_kind(internal_kind::int64);
    return impl_.repr.int64;
}

BHO_CXX14_CONSTEXPR std::uint64_t bho::mysql::field_view::as_uint64() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<std::uint64_t>();
    check_kind(internal_kind::uint64);
    return impl_.repr.uint64;
}

BHO_CXX14_CONSTEXPR bho::mysql::string_view bho::mysql::field_view::as_string() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<std::string>();
    check_kind(internal_kind::string);
    return impl_.repr.string;
}

BHO_CXX14_CONSTEXPR bho::mysql::blob_view bho::mysql::field_view::as_blob() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<blob>();
    check_kind(internal_kind::blob);
    return impl_.repr.blob;
}

BHO_CXX14_CONSTEXPR float bho::mysql::field_view::as_float() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<float>();
    check_kind(internal_kind::float_);
    return impl_.repr.float_;
}

BHO_CXX14_CONSTEXPR double bho::mysql::field_view::as_double() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<double>();
    check_kind(internal_kind::double_);
    return impl_.repr.double_;
}

BHO_CXX14_CONSTEXPR bho::mysql::date bho::mysql::field_view::as_date() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<date>();
    check_kind(internal_kind::date);
    return impl_.repr.date_;
}

BHO_CXX14_CONSTEXPR bho::mysql::datetime bho::mysql::field_view::as_datetime() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<datetime>();
    check_kind(internal_kind::datetime);
    return impl_.repr.datetime_;
}

BHO_CXX14_CONSTEXPR bho::mysql::time bho::mysql::field_view::as_time() const
{
    if (is_field_ptr())
        return impl_.repr.field_ptr->as<time>();
    check_kind(internal_kind::time);
    return impl_.repr.time_;
}

BHO_CXX14_CONSTEXPR void bho::mysql::field_view::check_kind(internal_kind expected) const
{
    if (impl_.ikind != expected)
        BHO_THROW_EXCEPTION(bad_field_access());
}

BHO_CXX14_CONSTEXPR bool bho::mysql::field_view::operator==(const field_view& rhs) const noexcept
{
    // Make operator== work for types not representable by field_kind
    if (impl_.ikind == internal_kind::sv_offset_string || impl_.ikind == internal_kind::sv_offset_blob)
    {
        return rhs.impl_.ikind == impl_.ikind && impl_.repr.sv_offset_ == rhs.impl_.repr.sv_offset_;
    }

    auto k = kind(), rhs_k = rhs.kind();
    switch (k)
    {
    case field_kind::null: return rhs_k == field_kind::null;
    case field_kind::int64:
        if (rhs_k == field_kind::int64)
            return get_int64() == rhs.get_int64();
        else if (rhs_k == field_kind::uint64)
        {
            std::int64_t this_val = get_int64();
            if (this_val < 0)
                return false;
            else
                return static_cast<std::uint64_t>(this_val) == rhs.get_uint64();
        }
        else
            return false;
    case field_kind::uint64:
        if (rhs_k == field_kind::uint64)
            return get_uint64() == rhs.get_uint64();
        else if (rhs_k == field_kind::int64)
        {
            std::int64_t rhs_val = rhs.get_int64();
            if (rhs_val < 0)
                return false;
            else
                return static_cast<std::uint64_t>(rhs_val) == get_uint64();
        }
        else
            return false;
    case field_kind::string: return rhs_k == field_kind::string && get_string() == rhs.get_string();
    case field_kind::blob:
        return rhs_k == field_kind::blob && detail::blobs_equal(get_blob(), rhs.get_blob());
    case field_kind::float_: return rhs_k == field_kind::float_ && get_float() == rhs.get_float();
    case field_kind::double_: return rhs_k == field_kind::double_ && get_double() == rhs.get_double();
    case field_kind::date: return rhs_k == field_kind::date && get_date() == rhs.get_date();
    case field_kind::datetime: return rhs_k == field_kind::datetime && get_datetime() == rhs.get_datetime();
    case field_kind::time: return rhs_k == field_kind::time && get_time() == rhs.get_time();
    default: BHO_ASSERT(false); return false;
    }
}

#endif
