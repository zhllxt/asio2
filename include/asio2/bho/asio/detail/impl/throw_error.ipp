#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/throw_error.ipp>
#else
#include <boost/asio/detail/impl/throw_error.ipp>
#endif
