#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/random_access_file.hpp>
#else
#include <boost/asio/random_access_file.hpp>
#endif
