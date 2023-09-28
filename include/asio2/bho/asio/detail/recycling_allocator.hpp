#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/recycling_allocator.hpp>
#else
#include <boost/asio/detail/recycling_allocator.hpp>
#endif
