#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/handler_cont_helpers.hpp>
#else
#include <boost/asio/detail/handler_cont_helpers.hpp>
#endif
