#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ts/netfwd.hpp>
#else
#include <boost/asio/ts/netfwd.hpp>
#endif
