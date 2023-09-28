#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/socket_ops.hpp>
#else
#include <boost/asio/detail/socket_ops.hpp>
#endif
