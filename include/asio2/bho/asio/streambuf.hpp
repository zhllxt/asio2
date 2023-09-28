#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/streambuf.hpp>
#else
#include <boost/asio/streambuf.hpp>
#endif
