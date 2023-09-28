#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/impl/win_iocp_handle_service.ipp>
#else
#include <boost/asio/detail/impl/win_iocp_handle_service.ipp>
#endif
