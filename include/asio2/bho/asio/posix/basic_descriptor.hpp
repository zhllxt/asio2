#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/posix/basic_descriptor.hpp>
#else
#include <boost/asio/posix/basic_descriptor.hpp>
#endif
