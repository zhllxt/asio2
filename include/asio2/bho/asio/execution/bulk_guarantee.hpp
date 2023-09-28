#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/bulk_guarantee.hpp>
#else
#include <boost/asio/execution/bulk_guarantee.hpp>
#endif
