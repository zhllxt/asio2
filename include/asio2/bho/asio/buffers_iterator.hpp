#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/buffers_iterator.hpp>
#else
#include <boost/asio/buffers_iterator.hpp>
#endif
