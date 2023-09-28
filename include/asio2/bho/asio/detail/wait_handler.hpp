#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/wait_handler.hpp>
#else
#include <boost/asio/detail/wait_handler.hpp>
#endif
