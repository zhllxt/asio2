#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/signal_set_service.ipp>
#else
#include <boost/asio/detail/impl/signal_set_service.ipp>
#endif
