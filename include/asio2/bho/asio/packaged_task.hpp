#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/packaged_task.hpp>
#else
#include <boost/asio/packaged_task.hpp>
#endif
