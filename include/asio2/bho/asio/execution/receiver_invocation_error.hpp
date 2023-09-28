#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/receiver_invocation_error.hpp>
#else
#include <boost/asio/execution/receiver_invocation_error.hpp>
#endif
