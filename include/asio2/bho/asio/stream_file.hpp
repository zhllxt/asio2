#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/stream_file.hpp>
#else
#include <boost/asio/stream_file.hpp>
#endif
