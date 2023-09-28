#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/date_time_fwd.hpp>
#else
#include <boost/asio/detail/date_time_fwd.hpp>
#endif
