#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/experimental/detail/channel_send_functions.hpp>
#else
#include <boost/asio/experimental/detail/channel_send_functions.hpp>
#endif
