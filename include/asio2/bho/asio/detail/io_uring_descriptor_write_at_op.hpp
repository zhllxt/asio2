#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/io_uring_descriptor_write_at_op.hpp>
#else
#include <boost/asio/detail/io_uring_descriptor_write_at_op.hpp>
#endif
