#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/null_event.ipp>
#else
#include <boost/asio/detail/impl/null_event.ipp>
#endif
