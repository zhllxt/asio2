#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/steady_timer.hpp>
#else
#include <boost/asio/steady_timer.hpp>
#endif
