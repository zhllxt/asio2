//
// Copyright (c) 2019 Damian Jarek(damian.jarek93@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_DETAIL_TEMPORARY_BUFFER_HPP
#define BEAST_DETAIL_TEMPORARY_BUFFER_HPP

#include <beast/core/detail/config.hpp>
#include <beast/core/string.hpp>

#include <memory>

namespace beast {
namespace detail {

struct temporary_buffer
{
    temporary_buffer() = default;
    temporary_buffer(temporary_buffer const&) = delete;
    temporary_buffer& operator=(temporary_buffer const&) = delete;

    ~temporary_buffer() noexcept
    {
        deallocate(data_);
    }

    BEAST_DECL
    void
    append(string_view s);

    BEAST_DECL
    void
    append(string_view s1, string_view s2);

    string_view
    view() const noexcept
    {
        return {data_, size_};
    }

    bool
    empty() const noexcept
    {
        return size_ == 0;
    }

private:
    BEAST_DECL
    void
    unchecked_append(string_view s);

    BEAST_DECL
    void
    grow(std::size_t n);

    void
    deallocate(char* data) noexcept
    {
        if (data != buffer_)
            delete[] data;
    }

    char buffer_[4096];
    char* data_ = buffer_;
    std::size_t capacity_ = sizeof(buffer_);
    std::size_t size_ = 0;
};

} // detail
} // beast

#ifdef BEAST_HEADER_ONLY
#include <beast/core/detail/impl/temporary_buffer.ipp>
#endif

#endif
