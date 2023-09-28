#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/compose.hpp>
#else
#include <boost/asio/impl/compose.hpp>
#endif
