#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/experimental/impl/channel_error.ipp>
#else
#include <boost/asio/experimental/impl/channel_error.ipp>
#endif
