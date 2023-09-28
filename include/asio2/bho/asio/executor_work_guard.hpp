#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/executor_work_guard.hpp>
#else
#include <boost/asio/executor_work_guard.hpp>
#endif
