#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/async_result.hpp>
#else
#include <boost/asio/async_result.hpp>
#endif
