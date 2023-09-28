#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/cancellation_signal.hpp>
#else
#include <boost/asio/cancellation_signal.hpp>
#endif
