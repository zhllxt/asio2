#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/traits/query_static_constexpr_member.hpp>
#else
#include <boost/asio/traits/query_static_constexpr_member.hpp>
#endif
