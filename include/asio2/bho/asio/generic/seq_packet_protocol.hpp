#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/generic/seq_packet_protocol.hpp>
#else
#include <boost/asio/generic/seq_packet_protocol.hpp>
#endif
