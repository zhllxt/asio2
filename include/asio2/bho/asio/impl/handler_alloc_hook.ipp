#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/handler_alloc_hook.ipp>
#else
#include <boost/asio/impl/handler_alloc_hook.ipp>
#endif
