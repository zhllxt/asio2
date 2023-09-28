#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/signal_set.hpp>
#else
#include <boost/asio/signal_set.hpp>
#endif
