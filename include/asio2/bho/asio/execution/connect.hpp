#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/connect.hpp>
#else
#include <boost/asio/execution/connect.hpp>
#endif
