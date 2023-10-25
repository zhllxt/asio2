//
// Copyright (c) 2022 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_BEAST_CORE_DETAIL_FILTERING_CANCELLATION_SLOT_HPP
#define BHO_BEAST_CORE_DETAIL_FILTERING_CANCELLATION_SLOT_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio/cancellation_signal.hpp>

namespace bho {
namespace beast {
namespace detail {

template<typename CancellationSlot = net::cancellation_slot>
struct filtering_cancellation_slot : CancellationSlot
{
    template<typename ... Args>
    filtering_cancellation_slot(net::cancellation_type type, Args && ... args)
            : CancellationSlot(std::forward<Args>(args)...), type(type) {}

    net::cancellation_type type = net::cancellation_type::terminal;

    using CancellationSlot::operator=;

    template<typename Handler>
    struct handler_wrapper
    {
        Handler handler;
        const net::cancellation_type type;

        template<typename ... Args>
        handler_wrapper(net::cancellation_type type, Args && ... args)
                : handler(std::forward<Args>(args)...),
                  type(type) {}

        void operator()(net::cancellation_type tp)
        {
            if ((tp & type) != net::cancellation_type::none)
                handler(tp);
        }
    };

    template <typename CancellationHandler, typename ... Args>
    CancellationHandler& emplace(Args && ... args)
    {
        return CancellationSlot::template emplace<handler_wrapper<CancellationHandler>>(
                type, std::forward<Args>(args)...).handler;
    }

    template <typename CancellationHandler>
    CancellationHandler& assign(CancellationHandler && ch)
    {
        return CancellationSlot::template emplace<handler_wrapper<CancellationHandler>>(
                type, std::forward<CancellationHandler>(ch)).handler;
    }
};

}
}
}
#endif //BHO_BEAST_CORE_DETAIL_FILTERING_CANCELLATION_SLOT_HPP
