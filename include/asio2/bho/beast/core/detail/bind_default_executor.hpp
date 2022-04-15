//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_DETAIL_BIND_DEFAULT_EXECUTOR_HPP
#define BHO_BEAST_CORE_DETAIL_BIND_DEFAULT_EXECUTOR_HPP

#include <asio2/external/asio.hpp>
#include <asio2/bho/core/empty_value.hpp>
#include <utility>

namespace bho {
namespace beast {
namespace detail {

template<class Handler, class Executor>
class bind_default_executor_wrapper
    : private bho::empty_value<Executor>
{
    Handler h_;

public:
    template<class Handler_>
    bind_default_executor_wrapper(
        Handler_&& h,
        Executor const& ex)
        : bho::empty_value<Executor>(
            bho::empty_init_t{}, ex)
        , h_(std::forward<Handler_>(h))
    {
    }

    template<class... Args>
    void
    operator()(Args&&... args)
    {
        h_(std::forward<Args>(args)...);
    }

    using allocator_type =
        net::associated_allocator_t<Handler>;

    allocator_type
    get_allocator() const noexcept
    {
        return net::get_associated_allocator(h_);
    }

    using executor_type =
        net::associated_executor_t<Handler, Executor>;

    executor_type
    get_executor() const noexcept
    {
        return net::get_associated_executor(
            h_, this->get());
    }

    // The allocation hooks are still defined because they trivially forward to
    // user hooks. Forward here ensures that the user will get a compile error
    // if they build their code with ASIO_NO_DEPRECATED.

    friend
    net::asio_handler_allocate_is_deprecated
    asio_handler_allocate(
        std::size_t size, bind_default_executor_wrapper* p)
    {
        using net::asio_handler_allocate;
        return asio_handler_allocate(
            size, std::addressof(p->h_));
    }

    friend
    net::asio_handler_deallocate_is_deprecated
    asio_handler_deallocate(
        void* mem, std::size_t size,
            bind_default_executor_wrapper* p)
    {
        using net::asio_handler_deallocate;
        return asio_handler_deallocate(mem, size,
            std::addressof(p->h_));
    }

    friend
    bool asio_handler_is_continuation(
        bind_default_executor_wrapper* p)
    {
        using net::asio_handler_is_continuation;
        return asio_handler_is_continuation(
            std::addressof(p->h_));
    }
};

template<class Executor, class Handler>
auto
bind_default_executor(Executor const& ex, Handler&& h) ->
    bind_default_executor_wrapper<
        typename std::decay<Handler>::type,
        Executor>
{
    return bind_default_executor_wrapper<
        typename std::decay<Handler>::type, 
            Executor>(std::forward<Handler>(h), ex);
}

} // detail
} // beast
} // bho

#endif
