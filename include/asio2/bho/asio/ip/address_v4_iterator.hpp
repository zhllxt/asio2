#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/address_v4_iterator.hpp>
#else
#include <boost/asio/ip/address_v4_iterator.hpp>
#endif
