#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/any_executor.hpp>
#else
#include <boost/asio/execution/any_executor.hpp>
#endif
