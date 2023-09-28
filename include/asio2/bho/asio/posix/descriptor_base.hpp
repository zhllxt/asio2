#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/posix/descriptor_base.hpp>
#else
#include <boost/asio/posix/descriptor_base.hpp>
#endif
