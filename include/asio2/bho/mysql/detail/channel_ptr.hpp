//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_DETAIL_CHANNEL_PTR_HPP
#define BHO_MYSQL_DETAIL_CHANNEL_PTR_HPP

#include <asio2/bho/mysql/diagnostics.hpp>
#include <asio2/bho/mysql/field_view.hpp>
#include <asio2/bho/mysql/metadata_mode.hpp>

#include <asio2/bho/mysql/detail/any_stream.hpp>
#include <asio2/bho/mysql/detail/config.hpp>

#include <asio2/bho/assert.hpp>

#include <memory>

namespace bho {
namespace mysql {
namespace detail {

class channel;

class channel_ptr
{
    std::unique_ptr<channel> chan_;

    BHO_MYSQL_DECL any_stream& get_stream() const;

public:
    BHO_MYSQL_DECL channel_ptr(std::size_t read_buff_size, std::unique_ptr<any_stream>);
    channel_ptr(const channel_ptr&) = delete;
    BHO_MYSQL_DECL channel_ptr(channel_ptr&&) noexcept;
    channel_ptr& operator=(const channel_ptr&) = delete;
    BHO_MYSQL_DECL channel_ptr& operator=(channel_ptr&&) noexcept;
    BHO_MYSQL_DECL ~channel_ptr();

    any_stream& stream() noexcept { return get_stream(); }
    const any_stream& stream() const noexcept { return get_stream(); }

    channel& get() noexcept
    {
        BHO_ASSERT(chan_);
        return *chan_;
    }
    const channel& get() const noexcept
    {
        BHO_ASSERT(chan_);
        return *chan_;
    }

    BHO_MYSQL_DECL metadata_mode meta_mode() const noexcept;
    BHO_MYSQL_DECL void set_meta_mode(metadata_mode v) noexcept;
    BHO_MYSQL_DECL diagnostics& shared_diag() noexcept;
};

BHO_MYSQL_DECL std::vector<field_view>& get_shared_fields(channel&) noexcept;

}  // namespace detail
}  // namespace mysql
}  // namespace bho

#ifdef BHO_MYSQL_HEADER_ONLY
#include <asio2/bho/mysql/impl/channel_ptr.ipp>
#endif

#endif
