#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/execution/invocable_archetype.hpp>
#else
#include <boost/asio/execution/invocable_archetype.hpp>
#endif
