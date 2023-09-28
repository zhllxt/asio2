#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/use_awaitable.hpp>
#else
#include <boost/asio/use_awaitable.hpp>
#endif
