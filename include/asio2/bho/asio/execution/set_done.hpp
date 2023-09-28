#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/set_done.hpp>
#else
#include <boost/asio/execution/set_done.hpp>
#endif
