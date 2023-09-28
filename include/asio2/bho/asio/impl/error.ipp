#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/error.ipp>
#else
#include <boost/asio/impl/error.ipp>
#endif
