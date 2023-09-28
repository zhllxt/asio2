#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/impl/address.hpp>
#else
#include <boost/asio/ip/impl/address.hpp>
#endif
