#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/detail/variadic_templates.hpp>
#else
#include <boost/asio/detail/variadic_templates.hpp>
#endif
