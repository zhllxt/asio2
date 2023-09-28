#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/host_name_verification.hpp>
#else
#include <boost/asio/ssl/host_name_verification.hpp>
#endif
