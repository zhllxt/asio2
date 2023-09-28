#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/socket_base.hpp>
#else
#include <boost/asio/socket_base.hpp>
#endif
