#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/conditionally_enabled_mutex.hpp>
#else
#include <boost/asio/detail/conditionally_enabled_mutex.hpp>
#endif
