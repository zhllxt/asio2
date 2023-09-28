#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/string_view.hpp>
#else
#include <boost/asio/detail/string_view.hpp>
#endif
