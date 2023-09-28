#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/thread_pool.hpp>
#else
#include <boost/asio/impl/thread_pool.hpp>
#endif
