#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/consuming_buffers.hpp>
#else
#include <boost/asio/detail/consuming_buffers.hpp>
#endif
