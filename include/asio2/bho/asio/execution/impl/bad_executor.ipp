#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/impl/bad_executor.ipp>
#else
#include <boost/asio/execution/impl/bad_executor.ipp>
#endif
