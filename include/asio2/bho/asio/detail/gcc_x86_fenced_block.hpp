#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/gcc_x86_fenced_block.hpp>
#else
#include <boost/asio/detail/gcc_x86_fenced_block.hpp>
#endif
