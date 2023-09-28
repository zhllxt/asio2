#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/bind_executor.hpp>
#else
#include <boost/asio/bind_executor.hpp>
#endif
