#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/traits/equality_comparable.hpp>
#else
#include <boost/asio/traits/equality_comparable.hpp>
#endif
