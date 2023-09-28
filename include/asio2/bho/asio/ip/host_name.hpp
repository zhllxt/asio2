#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/host_name.hpp>
#else
#include <boost/asio/ip/host_name.hpp>
#endif
