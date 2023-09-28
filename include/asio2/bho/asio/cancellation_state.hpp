#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/cancellation_state.hpp>
#else
#include <boost/asio/cancellation_state.hpp>
#endif
