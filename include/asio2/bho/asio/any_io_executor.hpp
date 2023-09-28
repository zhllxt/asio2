#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/any_io_executor.hpp>
#else
#include <boost/asio/any_io_executor.hpp>
#endif
