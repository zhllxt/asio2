#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/serial_port_base.ipp>
#else
#include <boost/asio/impl/serial_port_base.ipp>
#endif
