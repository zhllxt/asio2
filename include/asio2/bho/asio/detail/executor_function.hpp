#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/executor_function.hpp>
#else
#include <boost/asio/detail/executor_function.hpp>
#endif
