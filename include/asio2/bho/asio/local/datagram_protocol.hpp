#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/local/datagram_protocol.hpp>
#else
#include <boost/asio/local/datagram_protocol.hpp>
#endif
