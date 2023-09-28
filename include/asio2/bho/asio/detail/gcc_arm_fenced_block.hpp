#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/gcc_arm_fenced_block.hpp>
#else
#include <boost/asio/detail/gcc_arm_fenced_block.hpp>
#endif
