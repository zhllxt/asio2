#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/experimental/detail/coro_promise_allocator.hpp>
#else
#include <boost/asio/experimental/detail/coro_promise_allocator.hpp>
#endif
