#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/buffer_sequence_adapter.ipp>
#else
#include <boost/asio/detail/impl/buffer_sequence_adapter.ipp>
#endif
