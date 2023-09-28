#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/redirect_error.hpp>
#else
#include <boost/asio/redirect_error.hpp>
#endif
