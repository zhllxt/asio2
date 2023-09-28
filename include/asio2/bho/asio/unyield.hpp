#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/unyield.hpp>
#else
#include <boost/asio/unyield.hpp>
#endif
