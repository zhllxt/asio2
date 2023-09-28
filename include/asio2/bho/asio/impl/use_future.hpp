#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/use_future.hpp>
#else
#include <boost/asio/impl/use_future.hpp>
#endif
