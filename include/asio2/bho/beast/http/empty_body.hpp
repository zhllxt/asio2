//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_HTTP_EMPTY_BODY_HPP
#define BHO_BEAST_HTTP_EMPTY_BODY_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/beast/http/error.hpp>
#include <asio2/bho/beast/http/message.hpp>
#include <optional>

#include <cstdint>

namespace bho {
namespace beast {
namespace http {

/** An empty <em>Body</em>

    This body is used to represent messages which do not have a
    message body. If this body is used with a parser, and the
    parser encounters octets corresponding to a message body,
    the parser will fail with the error @ref http::unexpected_body.

    The Content-Length of this body is always 0.
*/
struct empty_body
{
    /** The type of container used for the body

        This determines the type of @ref message::body
        when this body type is used with a message container.
    */
    struct value_type
    {
    };

    /** Returns the payload size of the body

        When this body is used with @ref message::prepare_payload,
        the Content-Length will be set to the payload size, and
        any chunked Transfer-Encoding will be removed.
    */
    static
    std::uint64_t
    size(value_type)
    {
        return 0;
    }

    /** The algorithm for parsing the body

        Meets the requirements of <em>BodyReader</em>.
    */
#if BHO_BEAST_DOXYGEN
    using reader = __implementation_defined__;
#else
    struct reader
    {
        template<bool isRequest, class Fields>
        explicit
        reader(header<isRequest, Fields>&, value_type&)
        {
        }

        void
        init(std::optional<std::uint64_t> const&, error_code& ec)
        {
            ec = {};
        }

        template<class ConstBufferSequence>
        std::size_t
        put(ConstBufferSequence const&,
            error_code& ec)
        {
            BHO_BEAST_ASSIGN_EC(ec, error::unexpected_body);
            return 0;
        }

        void
        finish(error_code& ec)
        {
            ec = {};
        }
    };
#endif

    /** The algorithm for serializing the body

        Meets the requirements of <em>BodyWriter</em>.
    */
#if BHO_BEAST_DOXYGEN
    using writer = __implementation_defined__;
#else
    struct writer
    {
        using const_buffers_type =
            net::const_buffer;

        template<bool isRequest, class Fields>
        explicit
        writer(header<isRequest, Fields> const&, value_type const&)
        {
        }

        void
        init(error_code& ec)
        {
            ec = {};
        }

        std::optional<std::pair<const_buffers_type, bool>>
        get(error_code& ec)
        {
            ec = {};
            return std::nullopt;
        }
    };
#endif
};

} // http
} // beast
} // bho

#endif
