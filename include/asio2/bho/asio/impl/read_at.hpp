#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/read_at.hpp>
#else
#include <boost/asio/impl/read_at.hpp>
#endif
