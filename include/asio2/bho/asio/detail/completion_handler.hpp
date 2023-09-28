#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/completion_handler.hpp>
#else
#include <boost/asio/detail/completion_handler.hpp>
#endif
