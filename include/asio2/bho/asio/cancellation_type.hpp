#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/cancellation_type.hpp>
#else
#include <boost/asio/cancellation_type.hpp>
#endif
