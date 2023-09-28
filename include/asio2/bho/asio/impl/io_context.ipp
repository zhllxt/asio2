#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/io_context.ipp>
#else
#include <boost/asio/impl/io_context.ipp>
#endif
