#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/v6_only.hpp>
#else
#include <boost/asio/ip/v6_only.hpp>
#endif
