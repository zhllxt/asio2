//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_IMPL_SAVED_HANDLER_HPP
#define BHO_BEAST_CORE_IMPL_SAVED_HANDLER_HPP

#include <asio2/bho/beast/core/detail/allocator.hpp>
#include <asio2/external/asio.hpp>
#include <asio2/bho/assert.hpp>
#include <asio2/bho/core/empty_value.hpp>
#include <asio2/bho/core/exchange.hpp>
#include <utility>

namespace bho {
namespace beast {

//------------------------------------------------------------------------------

class saved_handler::base
{
protected:
    ~base() = default;

public:
    base() = default;
    virtual void destroy() = 0;
    virtual void invoke() = 0;
};

//------------------------------------------------------------------------------

template<class Handler, class Alloc>
class saved_handler::impl final : public base
{
    using alloc_type = typename
        beast::detail::allocator_traits<
            Alloc>::template rebind_alloc<impl>;

    using alloc_traits =
        beast::detail::allocator_traits<alloc_type>;

    struct ebo_pair : bho::empty_value<alloc_type>
    {
        Handler h;

        template<class Handler_>
        ebo_pair(
            alloc_type const& a,
            Handler_&& h_)
            : bho::empty_value<alloc_type>(
                bho::empty_init_t{}, a)
            , h(std::forward<Handler_>(h_))
        {
        }
    };

    ebo_pair v_;
#if defined(ASIO_NO_TS_EXECUTORS)
    typename std::decay<decltype(net::prefer(std::declval<
        net::associated_executor_t<Handler>>(),
        net::execution::outstanding_work.tracked))>::type
          wg2_;
#else // defined(ASIO_NO_TS_EXECUTORS)
    net::executor_work_guard<
        net::associated_executor_t<Handler>> wg2_;
#endif // defined(ASIO_NO_TS_EXECUTORS)

public:
    template<class Handler_>
    impl(alloc_type const& a, Handler_&& h)
        : v_(a, std::forward<Handler_>(h))
#if defined(ASIO_NO_TS_EXECUTORS)
        , wg2_(net::prefer(
            net::get_associated_executor(v_.h),
            net::execution::outstanding_work.tracked))
#else // defined(ASIO_NO_TS_EXECUTORS)
        , wg2_(net::get_associated_executor(v_.h))
#endif // defined(ASIO_NO_TS_EXECUTORS)
    {
    }

    void
    destroy() override
    {
        auto v = std::move(v_);
        alloc_traits::destroy(v.get(), this);
        alloc_traits::deallocate(v.get(), this, 1);
    }

    void
    invoke() override
    {
        auto v = std::move(v_);
        alloc_traits::destroy(v.get(), this);
        alloc_traits::deallocate(v.get(), this, 1);
        v.h();
    }
};

//------------------------------------------------------------------------------

template<class Handler, class Allocator>
void
saved_handler::
emplace(Handler&& handler, Allocator const& alloc)
{
    // Can't delete a handler before invoking
    BHO_ASSERT(! has_value());
    using handler_type =
        typename std::decay<Handler>::type;
    using alloc_type = typename
        detail::allocator_traits<Allocator>::
            template rebind_alloc<impl<
                handler_type, Allocator>>;
    using alloc_traits =
        beast::detail::allocator_traits<alloc_type>;
    struct storage
    {
        alloc_type a;
        impl<Handler, Allocator>* p;

        explicit
        storage(Allocator const& a_)
            : a(a_)
            , p(alloc_traits::allocate(a, 1))
        {
        }

        ~storage()
        {
            if(p)
                alloc_traits::deallocate(a, p, 1);
        }
    };
    storage s(alloc);
    alloc_traits::construct(s.a, s.p,
        s.a, std::forward<Handler>(handler));
    p_ = std::exchange(s.p, nullptr);
}

template<class Handler>
void
saved_handler::
emplace(Handler&& handler)
{
    // Can't delete a handler before invoking
    BHO_ASSERT(! has_value());
    emplace(
        std::forward<Handler>(handler),
        net::get_associated_allocator(handler));
}

} // beast
} // bho

#endif
