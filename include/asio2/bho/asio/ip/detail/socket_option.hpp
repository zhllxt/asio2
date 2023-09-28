#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/detail/socket_option.hpp>
#else
#include <boost/asio/ip/detail/socket_option.hpp>
#endif
