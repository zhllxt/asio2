#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/functional.hpp>
#else
#include <boost/asio/detail/functional.hpp>
#endif
