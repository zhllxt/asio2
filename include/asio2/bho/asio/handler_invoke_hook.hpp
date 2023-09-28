#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/handler_invoke_hook.hpp>
#else
#include <boost/asio/handler_invoke_hook.hpp>
#endif
