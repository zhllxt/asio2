#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/this_coro.hpp>
#else
#include <boost/asio/this_coro.hpp>
#endif
