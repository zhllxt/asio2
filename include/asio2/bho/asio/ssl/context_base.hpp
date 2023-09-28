#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/context_base.hpp>
#else
#include <boost/asio/ssl/context_base.hpp>
#endif
