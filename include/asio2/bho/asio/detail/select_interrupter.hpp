#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/select_interrupter.hpp>
#else
#include <boost/asio/detail/select_interrupter.hpp>
#endif
