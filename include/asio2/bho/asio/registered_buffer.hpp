#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/registered_buffer.hpp>
#else
#include <boost/asio/registered_buffer.hpp>
#endif
