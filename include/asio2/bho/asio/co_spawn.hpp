#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/co_spawn.hpp>
#else
#include <boost/asio/co_spawn.hpp>
#endif
