#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/bad_address_cast.hpp>
#else
#include <boost/asio/ip/bad_address_cast.hpp>
#endif
