#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ts/buffer.hpp>
#else
#include <boost/asio/ts/buffer.hpp>
#endif
