#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/traits/query_member.hpp>
#else
#include <boost/asio/traits/query_member.hpp>
#endif
