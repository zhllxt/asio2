#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/old_win_sdk_compat.hpp>
#else
#include <boost/asio/detail/old_win_sdk_compat.hpp>
#endif
