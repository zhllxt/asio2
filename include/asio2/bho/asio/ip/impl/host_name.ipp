#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/impl/host_name.ipp>
#else
#include <boost/asio/ip/impl/host_name.ipp>
#endif
