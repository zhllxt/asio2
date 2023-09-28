#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/local/detail/endpoint.hpp>
#else
#include <boost/asio/local/detail/endpoint.hpp>
#endif
