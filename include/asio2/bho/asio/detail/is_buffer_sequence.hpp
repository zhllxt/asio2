#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/is_buffer_sequence.hpp>
#else
#include <boost/asio/detail/is_buffer_sequence.hpp>
#endif
