#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/cancellation_signal.ipp>
#else
#include <boost/asio/impl/cancellation_signal.ipp>
#endif
