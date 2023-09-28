#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/dependent_type.hpp>
#else
#include <boost/asio/detail/dependent_type.hpp>
#endif
