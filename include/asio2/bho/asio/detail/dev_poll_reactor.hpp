#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/dev_poll_reactor.hpp>
#else
#include <boost/asio/detail/dev_poll_reactor.hpp>
#endif
