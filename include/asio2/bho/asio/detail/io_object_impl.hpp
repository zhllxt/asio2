#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/io_object_impl.hpp>
#else
#include <boost/asio/detail/io_object_impl.hpp>
#endif
