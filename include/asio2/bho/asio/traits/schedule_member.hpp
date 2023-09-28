#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/traits/schedule_member.hpp>
#else
#include <boost/asio/traits/schedule_member.hpp>
#endif
