#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/source_location.hpp>
#else
#include <boost/asio/detail/source_location.hpp>
#endif
