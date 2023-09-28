#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ip/basic_resolver_entry.hpp>
#else
#include <boost/asio/ip/basic_resolver_entry.hpp>
#endif
