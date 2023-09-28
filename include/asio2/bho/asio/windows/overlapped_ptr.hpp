#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/windows/overlapped_ptr.hpp>
#else
#include <boost/asio/windows/overlapped_ptr.hpp>
#endif
