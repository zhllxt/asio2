#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/multiple_exceptions.hpp>
#else
#include <boost/asio/multiple_exceptions.hpp>
#endif
