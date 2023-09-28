#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/non_const_lvalue.hpp>
#else
#include <boost/asio/detail/non_const_lvalue.hpp>
#endif
