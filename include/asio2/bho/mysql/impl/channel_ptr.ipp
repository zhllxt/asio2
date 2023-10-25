//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_CHANNEL_PTR_IPP
#define BHO_MYSQL_IMPL_CHANNEL_PTR_IPP

#pragma once

#include <asio2/bho/mysql/detail/channel_ptr.hpp>

#include <asio2/bho/mysql/impl/internal/channel/channel.hpp>

bho::mysql::detail::channel_ptr::channel_ptr(std::size_t read_buff_size, std::unique_ptr<any_stream> stream)
    : chan_(new channel(read_buff_size, std::move(stream)))
{
}

bho::mysql::detail::channel_ptr::channel_ptr(channel_ptr&& rhs) noexcept : chan_(std::move(rhs.chan_)) {}

bho::mysql::detail::channel_ptr& bho::mysql::detail::channel_ptr::operator=(channel_ptr&& rhs) noexcept
{
    chan_ = std::move(rhs.chan_);
    return *this;
}

bho::mysql::detail::channel_ptr::~channel_ptr() {}

bho::mysql::detail::any_stream& bho::mysql::detail::channel_ptr::get_stream() const
{
    return chan_->stream();
}

bho::mysql::metadata_mode bho::mysql::detail::channel_ptr::meta_mode() const noexcept
{
    return chan_->meta_mode();
}

void bho::mysql::detail::channel_ptr::set_meta_mode(metadata_mode v) noexcept { chan_->set_meta_mode(v); }

bho::mysql::diagnostics& bho::mysql::detail::channel_ptr::shared_diag() noexcept
{
    return chan_->shared_diag();
}

std::vector<bho::mysql::field_view>& bho::mysql::detail::get_shared_fields(channel& chan) noexcept
{
    return chan.shared_fields();
}

#endif
