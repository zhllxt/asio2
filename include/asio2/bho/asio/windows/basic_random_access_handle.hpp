#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/windows/basic_random_access_handle.hpp>
#else
#include <boost/asio/windows/basic_random_access_handle.hpp>
#endif
