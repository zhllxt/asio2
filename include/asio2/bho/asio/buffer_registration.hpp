#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/buffer_registration.hpp>
#else
#include <boost/asio/buffer_registration.hpp>
#endif
