#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/descriptor_ops.ipp>
#else
#include <boost/asio/detail/impl/descriptor_ops.ipp>
#endif
