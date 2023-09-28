#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/execution_context.ipp>
#else
#include <boost/asio/impl/execution_context.ipp>
#endif
