#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/experimental/awaitable_operators.hpp>
#else
#include <boost/asio/experimental/awaitable_operators.hpp>
#endif
