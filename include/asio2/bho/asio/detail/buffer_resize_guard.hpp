#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/buffer_resize_guard.hpp>
#else
#include <boost/asio/detail/buffer_resize_guard.hpp>
#endif
