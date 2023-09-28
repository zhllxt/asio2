#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/ssl/impl/rfc2818_verification.ipp>
#else
#include <boost/asio/ssl/impl/rfc2818_verification.ipp>
#endif
