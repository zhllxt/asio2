#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/io_uring_descriptor_service.hpp>
#else
#include <boost/asio/detail/io_uring_descriptor_service.hpp>
#endif
