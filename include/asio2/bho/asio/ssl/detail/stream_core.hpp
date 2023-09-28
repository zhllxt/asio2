#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/detail/stream_core.hpp>
#else
#include <boost/asio/ssl/detail/stream_core.hpp>
#endif
