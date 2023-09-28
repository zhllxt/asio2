#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/static_thread_pool.hpp>
#else
#include <boost/asio/static_thread_pool.hpp>
#endif
