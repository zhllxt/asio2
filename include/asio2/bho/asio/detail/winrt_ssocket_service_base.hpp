#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/winrt_ssocket_service_base.hpp>
#else
#include <boost/asio/detail/winrt_ssocket_service_base.hpp>
#endif
