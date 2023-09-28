#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/array.hpp>
#else
#include <boost/asio/detail/array.hpp>
#endif
