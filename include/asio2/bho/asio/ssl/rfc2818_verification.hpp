#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/rfc2818_verification.hpp>
#else
#include <boost/asio/ssl/rfc2818_verification.hpp>
#endif
