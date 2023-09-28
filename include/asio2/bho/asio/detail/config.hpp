#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/config.hpp>
#else
#include <boost/asio/detail/config.hpp>
#endif
