#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/is_write_buffered.hpp>
#else
#include <boost/asio/is_write_buffered.hpp>
#endif
