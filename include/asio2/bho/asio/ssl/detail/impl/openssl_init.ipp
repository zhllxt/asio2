#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/detail/impl/openssl_init.ipp>
#else
#include <boost/asio/ssl/detail/impl/openssl_init.ipp>
#endif
