#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/reactive_null_buffers_op.hpp>
#else
#include <boost/asio/detail/reactive_null_buffers_op.hpp>
#endif
