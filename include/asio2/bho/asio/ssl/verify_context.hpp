#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/verify_context.hpp>
#else
#include <boost/asio/ssl/verify_context.hpp>
#endif
