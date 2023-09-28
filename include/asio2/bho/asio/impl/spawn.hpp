#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/impl/spawn.hpp>
#else
#include <boost/asio/impl/spawn.hpp>
#endif
