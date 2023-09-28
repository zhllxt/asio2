#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/base_from_cancellation_state.hpp>
#else
#include <boost/asio/detail/base_from_cancellation_state.hpp>
#endif
