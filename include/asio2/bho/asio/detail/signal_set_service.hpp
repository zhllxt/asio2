#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/signal_set_service.hpp>
#else
#include <boost/asio/detail/signal_set_service.hpp>
#endif
