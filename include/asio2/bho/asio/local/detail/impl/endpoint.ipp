#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/local/detail/impl/endpoint.ipp>
#else
#include <boost/asio/local/detail/impl/endpoint.ipp>
#endif
