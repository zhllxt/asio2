#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/detail/bulk_sender.hpp>
#else
#include <boost/asio/execution/detail/bulk_sender.hpp>
#endif
