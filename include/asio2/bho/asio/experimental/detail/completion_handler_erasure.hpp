#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/experimental/detail/completion_handler_erasure.hpp>
#else
#include <boost/asio/experimental/detail/completion_handler_erasure.hpp>
#endif
