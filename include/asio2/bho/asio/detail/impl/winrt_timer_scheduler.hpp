#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/winrt_timer_scheduler.hpp>
#else
#include <boost/asio/detail/impl/winrt_timer_scheduler.hpp>
#endif
