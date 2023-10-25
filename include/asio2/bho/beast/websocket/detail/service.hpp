//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_WEBSOCKET_DETAIL_SERVICE_HPP
#define BHO_BEAST_WEBSOCKET_DETAIL_SERVICE_HPP

#include <asio2/bho/beast/core/detail/service_base.hpp>
#include <asio/execution_context.hpp>
#include <mutex>
#include <vector>

namespace bho {
namespace beast {
namespace websocket {
namespace detail {

class service
    : public beast::detail::service_base<service>
{
public:
    class impl_type
        : public std::enable_shared_from_this<impl_type>
    {
        service& svc_;
        std::size_t index_;

        friend class service;

    public:
        virtual ~impl_type() = default;

        BHO_BEAST_DECL
        explicit
        impl_type(net::execution_context& ctx);

        BHO_BEAST_DECL
        void
        remove();

        virtual
        void
        shutdown() = 0;
    };

private:
    std::mutex m_;
    std::vector<impl_type*> v_;

    BHO_BEAST_DECL
    void
    shutdown() override;

public:
    BHO_BEAST_DECL
    explicit
    service(net::execution_context& ctx)
        : beast::detail::service_base<service>(ctx)
    {
    }
};

} // detail
} // websocket
} // beast
} // bho

#if BEAST_HEADER_ONLY
#include <asio2/bho/beast/websocket/detail/service.ipp>
#endif

#endif
