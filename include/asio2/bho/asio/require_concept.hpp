#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/require_concept.hpp>
#else
#include <boost/asio/require_concept.hpp>
#endif
