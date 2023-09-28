#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/win_iocp_socket_service.hpp>
#else
#include <boost/asio/detail/win_iocp_socket_service.hpp>
#endif
