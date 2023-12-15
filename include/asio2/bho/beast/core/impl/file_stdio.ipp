//
// Copyright (c) 2015-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_IMPL_FILE_STDIO_IPP
#define BHO_BEAST_CORE_IMPL_FILE_STDIO_IPP

#include <asio2/bho/beast/core/file_stdio.hpp>
#include <asio2/bho/config/workaround.hpp>
#include <asio2/bho/core/exchange.hpp>
#include <limits>

#if defined(BHO_MSVC)
#  pragma warning(push)
#  pragma warning(disable:4996) // warning C4996: 'fopen': This function or variable may be unsafe.
#endif

#if defined(BHO_GCC)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-variable"
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(BHO_CLANG)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wunused-variable"
#  pragma clang diagnostic ignored "-Wexceptions"
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#  pragma clang diagnostic ignored "-Wunused-private-field"
#  pragma clang diagnostic ignored "-Wunused-local-typedef"
#  pragma clang diagnostic ignored "-Wunknown-warning-option"
#endif

namespace bho {
namespace beast {

file_stdio::
~file_stdio()
{
    if(f_)
        f_.close();
}

file_stdio::
file_stdio(file_stdio&& other)
    : f_(std::move(other.f_))
{
}

file_stdio&
file_stdio::
operator=(file_stdio&& other)
{
    if(&other == this)
        return *this;
    if(f_)
        f_.close();
    f_ = std::move(other.f_);
    return *this;
}

void
file_stdio::
native_handle(std::fstream& f)
{
    if(f_)
        f_.close();
    f_ = std::move(f);
}

void
file_stdio::
close(error_code& ec)
{
    if(f_)
    {
        f_.close();
    }
    ec = {};
}

void
file_stdio::
open(char const* path, file_mode mode, error_code& ec)
{
    if(f_)
    {
        f_.close();
    }
    ec = {};
    std::ios_base::openmode s;
    switch(mode)
    {
    default:
    case file_mode::read:
        s = std::ios_base::in | std::ios_base::binary;
        break;

    case file_mode::scan:
        s = std::ios_base::in | std::ios_base::binary;
        break;

    case file_mode::write:
        s = std::ios_base::out | std::ios_base::binary;
        break;

    case file_mode::write_new:
    {
        s = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;
        break;
    }

    case file_mode::write_existing:
        s = std::ios_base::out | std::ios_base::binary;
        break;

    case file_mode::append:
        s = std::ios_base::out | std::ios_base::binary | std::ios_base::app;
        break;

    case file_mode::append_existing:
    {
        s = std::ios_base::out | std::ios_base::binary | std::ios_base::app;
        break;
    }
    }

    f_.open(path, s);
    if(! f_)
    {
        ec.assign(errno, generic_category());
        return;
    }
}

std::uint64_t
file_stdio::
size(error_code& ec) const
{
    if(! f_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    std::fstream::pos_type pos = f_.tellg();
    if(pos == std::fstream::pos_type(-1))
    {
        ec.assign(errno, generic_category());
        return 0;
    }
    auto& result = f_.seekg(0, std::ios_base::end);
    if(!result)
    {
        ec.assign(errno, generic_category());
        return 0;
    }
    std::fstream::pos_type size = f_.tellg();
    if(size == std::fstream::pos_type(-1))
    {
        ec.assign(errno, generic_category());
        f_.seekg(pos, std::ios_base::beg);
        return 0;
    }
    auto& result2 = f_.seekg(pos, std::ios_base::beg);
    if(!result2)
        ec.assign(errno, generic_category());
    else
        ec = {};
    return size;
}

std::uint64_t
file_stdio::
pos(error_code& ec) const
{
    if(! f_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    std::fstream::pos_type pos = f_.tellg();
    if(pos == std::fstream::pos_type(-1))
    {
        ec.assign(errno, generic_category());
        return 0;
    }
    ec = {};
    return pos;
}

void
file_stdio::
seek(std::uint64_t offset, error_code& ec)
{
    if(! f_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return;
    }
    if(offset > static_cast<std::uint64_t>((std::numeric_limits<long>::max)()))
    {
        ec = make_error_code(errc::invalid_seek);
        return;
    }
    auto& result = f_.seekg(offset, std::ios_base::beg);
    if(!result)
        ec.assign(errno, generic_category());
    else
        ec = {};
}

std::size_t
file_stdio::
read(void* buffer, std::size_t n, error_code& ec) const
{
    if(! f_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    auto& result = f_.read(reinterpret_cast<char*>(buffer), n);
    if(!result)
    {
        ec.assign(errno, generic_category());
        return 0;
    }
    return n;
}

std::size_t
file_stdio::
write(void const* buffer, std::size_t n, error_code& ec)
{
    if(! f_)
    {
        ec = make_error_code(errc::bad_file_descriptor);
        return 0;
    }
    auto& result = f_.write(reinterpret_cast<const char*>(buffer), n);
    if(!result)
    {
        ec.assign(errno, generic_category());
        return 0;
    }
    return n;
}

} // beast
} // bho

#if defined(BHO_CLANG)
#  pragma clang diagnostic pop
#endif

#if defined(BHO_GCC)
#  pragma GCC diagnostic pop
#endif

#if defined(BHO_MSVC)
#  pragma warning(pop)
#endif

#endif

