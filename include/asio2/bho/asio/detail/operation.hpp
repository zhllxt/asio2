#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/operation.hpp>
#else
#include <boost/asio/detail/operation.hpp>
#endif
