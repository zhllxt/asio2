#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/file_base.hpp>
#else
#include <boost/asio/file_base.hpp>
#endif
