#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/system_timer.hpp>
#else
#include <boost/asio/system_timer.hpp>
#endif
