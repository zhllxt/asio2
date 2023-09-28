#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/high_resolution_timer.hpp>
#else
#include <boost/asio/high_resolution_timer.hpp>
#endif
