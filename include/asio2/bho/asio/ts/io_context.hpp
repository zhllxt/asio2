#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ts/io_context.hpp>
#else
#include <boost/asio/ts/io_context.hpp>
#endif
