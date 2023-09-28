#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/buffer_sequence_adapter.hpp>
#else
#include <boost/asio/detail/buffer_sequence_adapter.hpp>
#endif
