#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/prefer_only.hpp>
#else
#include <boost/asio/execution/prefer_only.hpp>
#endif
