#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/icmp.hpp>
#else
#include <boost/asio/ip/icmp.hpp>
#endif
