#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/signal_handler.hpp>
#else
#include <boost/asio/detail/signal_handler.hpp>
#endif
