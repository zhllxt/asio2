#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/resolver_query_base.hpp>
#else
#include <boost/asio/ip/resolver_query_base.hpp>
#endif
