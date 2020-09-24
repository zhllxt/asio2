//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_HTTP_IMPL_SERIALIZER_HPP
#define BEAST_HTTP_IMPL_SERIALIZER_HPP

#include <beast/core/buffer_traits.hpp>
#include <beast/core/detail/buffers_ref.hpp>
#include <beast/http/error.hpp>
#include <beast/http/status.hpp>
#include <beast/core/detail/config.hpp>
#include <beast/core/util.hpp>
#include <ostream>

namespace beast {
namespace http {

template<
    bool isRequest, class Body, class Fields>
void
serializer<isRequest, Body, Fields>::
fwrinit(std::true_type)
{
    fwr_.emplace(m_, m_.version(), m_.method());
}

template<
    bool isRequest, class Body, class Fields>
void
serializer<isRequest, Body, Fields>::
fwrinit(std::false_type)
{
    fwr_.emplace(m_, m_.version(), m_.result_int());
}

template<
    bool isRequest, class Body, class Fields>
template<std::size_t I, class Visit>
inline
void
serializer<isRequest, Body, Fields>::
do_visit(error_code& ec, Visit& visit)
{
    pv_.template emplace<I-1>(limit_, &std::get<I-1>(v_));
    visit(ec, beast::detail::make_buffers_ref(
        std::get<I-1>(pv_)));
}

//------------------------------------------------------------------------------

template<
    bool isRequest, class Body, class Fields>
serializer<isRequest, Body, Fields>::
serializer(value_type& m)
    : m_(m)
    , wr_(this->m_.base(), this->m_.body())
{
}

template<
    bool isRequest, class Body, class Fields>
template<class Visit>
void
serializer<isRequest, Body, Fields>::
next(error_code& ec, Visit&& visit)
{
    switch(s_)
    {
    case do_construct:
    {
        fwrinit(std::integral_constant<bool,
            isRequest>{});
        if(m_.chunked())
            goto go_init_c;
        s_ = do_init;
        BEAST_FALLTHROUGH;
    }

    case do_init:
    {
        wr_.init(ec);
        if(ec)
            return;
        if(split_)
            goto go_header_only;
        auto result = wr_.get(ec);
        if(ec == error::need_more)
            goto go_header_only;
        if(ec)
            return;
        if(! result)
            goto go_header_only;
        more_ = result->second;
        v_.template emplace<2 - 1>(
            std::in_place,
            fwr_->get(),
            result->first);
        s_ = do_header;
        BEAST_FALLTHROUGH;
    }

    case do_header:
        do_visit<2>(ec, visit);
        break;

    go_header_only:
        v_.template emplace<1 - 1>(fwr_->get());
        s_ = do_header_only;
        BEAST_FALLTHROUGH;
    case do_header_only:
        do_visit<1>(ec, visit);
        break;

    case do_body:
        s_ = do_body + 1;
        BEAST_FALLTHROUGH;

    case do_body + 1:
    {
        auto result = wr_.get(ec);
        if(ec)
            return;
        if(! result)
            goto go_complete;
        more_ = result->second;
        v_.template emplace<3 - 1>(result->first);
        s_ = do_body + 2;
        BEAST_FALLTHROUGH;
    }

    case do_body + 2:
        do_visit<3>(ec, visit);
        break;

    //----------------------------------------------------------------------

        go_init_c:
        s_ = do_init_c;
        BEAST_FALLTHROUGH;
    case do_init_c:
    {
        wr_.init(ec);
        if(ec)
            return;
        if(split_)
            goto go_header_only_c;
        auto result = wr_.get(ec);
        if(ec == error::need_more)
            goto go_header_only_c;
        if(ec)
            return;
        if(! result)
            goto go_header_only_c;
        more_ = result->second;
        if(! more_)
        {
            // do it all in one buffer
            v_.template emplace<7 - 1>(
                std::in_place,
                fwr_->get(),
                buffer_bytes(result->first),
                net::const_buffer{nullptr, 0},
                chunk_crlf{},
                result->first,
                chunk_crlf{},
                detail::chunk_last(),
                net::const_buffer{nullptr, 0},
                chunk_crlf{});
            goto go_all_c;
        }
        v_.template emplace<4 - 1>(
            std::in_place,
            fwr_->get(),
            buffer_bytes(result->first),
            net::const_buffer{nullptr, 0},
            chunk_crlf{},
            result->first,
            chunk_crlf{});
        s_ = do_header_c;
        BEAST_FALLTHROUGH;
    }

    case do_header_c:
        do_visit<4>(ec, visit);
        break;

    go_header_only_c:
        v_.template emplace<1 - 1>(fwr_->get());
        s_ = do_header_only_c;
        BEAST_FALLTHROUGH;

    case do_header_only_c:
        do_visit<1>(ec, visit);
        break;

    case do_body_c:
        s_ = do_body_c + 1;
        BEAST_FALLTHROUGH;

    case do_body_c + 1:
    {
        auto result = wr_.get(ec);
        if(ec)
            return;
        if(! result)
            goto go_final_c;
        more_ = result->second;
        if(! more_)
        {
            // do it all in one buffer
            v_.template emplace<6 - 1>(
                std::in_place,
                buffer_bytes(result->first),
                net::const_buffer{nullptr, 0},
                chunk_crlf{},
                result->first,
                chunk_crlf{},
                detail::chunk_last(),
                net::const_buffer{nullptr, 0},
                chunk_crlf{});
            goto go_body_final_c;
        }
        v_.template emplace<5 - 1>(
            std::in_place,
            buffer_bytes(result->first),
            net::const_buffer{nullptr, 0},
            chunk_crlf{},
            result->first,
            chunk_crlf{});
        s_ = do_body_c + 2;
        BEAST_FALLTHROUGH;
    }

    case do_body_c + 2:
        do_visit<5>(ec, visit);
        break;

    go_body_final_c:
        s_ = do_body_final_c;
        BEAST_FALLTHROUGH;
    case do_body_final_c:
        do_visit<6>(ec, visit);
        break;

    go_all_c:
        s_ = do_all_c;
        BEAST_FALLTHROUGH;
    case do_all_c:
        do_visit<7>(ec, visit);
        break;

    go_final_c:
    case do_final_c:
        v_.template emplace<8 - 1>(
            std::in_place,
            detail::chunk_last(),
            net::const_buffer{nullptr, 0},
            chunk_crlf{});
        s_ = do_final_c + 1;
        BEAST_FALLTHROUGH;

    case do_final_c + 1:
        do_visit<8>(ec, visit);
        break;

    //----------------------------------------------------------------------

    default:
    case do_complete:
        BEAST_ASSERT(false);
        break;

    go_complete:
        s_ = do_complete;
        break;
    }
}

template<
    bool isRequest, class Body, class Fields>
void
serializer<isRequest, Body, Fields>::
consume(std::size_t n)
{
    switch(s_)
    {
    case do_header:
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<2-1>(v_)));
        std::get<2-1>(v_).consume(n);
        if(buffer_bytes(std::get<2-1>(v_)) > 0)
            break;
        header_done_ = true;
		v_.template emplace<1 - 1>();
        if(! more_)
            goto go_complete;
        s_ = do_body + 1;
        break;

    case do_header_only:
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<1-1>(v_)));
        std::get<1-1>(v_).consume(n);
        if(buffer_bytes(std::get<1-1>(v_)) > 0)
            break;
        fwr_ = std::nullopt;
        header_done_ = true;
        if(! split_)
            goto go_complete;
        s_ = do_body;
        break;

    case do_body + 2:
    {
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<3-1>(v_)));
        std::get<3-1>(v_).consume(n);
        if(buffer_bytes(std::get<3-1>(v_)) > 0)
            break;
        v_.template emplace<1 - 1>();
        if(! more_)
            goto go_complete;
        s_ = do_body + 1;
        break;
    }

    //----------------------------------------------------------------------

    case do_header_c:
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<4-1>(v_)));
        std::get<4-1>(v_).consume(n);
        if(buffer_bytes(std::get<4-1>(v_)) > 0)
            break;
        header_done_ = true;
        v_.template emplace<1 - 1>();
        if(more_)
            s_ = do_body_c + 1;
        else
            s_ = do_final_c;
        break;

    case do_header_only_c:
    {
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<1-1>(v_)));
        std::get<1-1>(v_).consume(n);
        if(buffer_bytes(std::get<1-1>(v_)) > 0)
            break;
        fwr_ = std::nullopt;
        header_done_ = true;
        if(! split_)
        {
            s_ = do_final_c;
            break;
        }
        s_ = do_body_c;
        break;
    }

    case do_body_c + 2:
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<5-1>(v_)));
        std::get<5-1>(v_).consume(n);
        if(buffer_bytes(std::get<5-1>(v_)) > 0)
            break;
        v_.template emplace<1 - 1>();
        if(more_)
            s_ = do_body_c + 1;
        else
            s_ = do_final_c;
        break;

    case do_body_final_c:
    {
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<6-1>(v_)));
        std::get<6-1>(v_).consume(n);
        if(buffer_bytes(std::get<6-1>(v_)) > 0)
            break;
        v_.template emplace<1 - 1>();
        s_ = do_complete;
        break;
    }

    case do_all_c:
    {
        BEAST_ASSERT(
            n <= buffer_bytes(std::get<7-1>(v_)));
        std::get<7-1>(v_).consume(n);
        if(buffer_bytes(std::get<7-1>(v_)) > 0)
            break;
        header_done_ = true;
        v_.template emplace<1 - 1>();
        s_ = do_complete;
        break;
    }

    case do_final_c + 1:
        BEAST_ASSERT(buffer_bytes(std::get<8-1>(v_)));
        std::get<8-1>(v_).consume(n);
        if(buffer_bytes(std::get<8-1>(v_)) > 0)
            break;
        v_.template emplace<1 - 1>();
        goto go_complete;

    //----------------------------------------------------------------------

    default:
        BEAST_ASSERT(false);
    case do_complete:
        break;

    go_complete:
        s_ = do_complete;
        break;
    }
}

} // http
} // beast

#endif
