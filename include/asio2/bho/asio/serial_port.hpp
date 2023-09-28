#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/serial_port.hpp>
#else
#include <boost/asio/serial_port.hpp>
#endif
