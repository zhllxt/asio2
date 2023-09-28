#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/win_fenced_block.hpp>
#else
#include <boost/asio/detail/win_fenced_block.hpp>
#endif
