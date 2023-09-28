#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/traits/static_require_concept.hpp>
#else
#include <boost/asio/traits/static_require_concept.hpp>
#endif
