#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/throw_exception.hpp>
#else
#include <boost/asio/detail/throw_exception.hpp>
#endif
