#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/uses_executor.hpp>
#else
#include <boost/asio/uses_executor.hpp>
#endif
