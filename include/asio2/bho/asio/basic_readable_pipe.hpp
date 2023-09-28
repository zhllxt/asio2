#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/basic_readable_pipe.hpp>
#else
#include <boost/asio/basic_readable_pipe.hpp>
#endif
