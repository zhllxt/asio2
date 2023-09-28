#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/impl/network_v6.ipp>
#else
#include <boost/asio/ip/impl/network_v6.ipp>
#endif
