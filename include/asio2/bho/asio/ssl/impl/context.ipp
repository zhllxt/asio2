#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/impl/context.ipp>
#else
#include <boost/asio/ssl/impl/context.ipp>
#endif
