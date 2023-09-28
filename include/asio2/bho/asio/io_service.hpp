#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/io_service.hpp>
#else
#include <boost/asio/io_service.hpp>
#endif
