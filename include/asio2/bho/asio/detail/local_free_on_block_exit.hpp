#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/local_free_on_block_exit.hpp>
#else
#include <boost/asio/detail/local_free_on_block_exit.hpp>
#endif
