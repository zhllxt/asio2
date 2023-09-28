#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/version.hpp>
#else
#include <boost/asio/version.hpp>
#endif
