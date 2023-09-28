#pragma once

#include <asio2/config.hpp>

#if defined(ASIO_STANDALONE) || defined(ASIO2_HEADER_ONLY)
#include <asio/associated_cancellation_slot.hpp>
#else
#include <boost/asio/associated_cancellation_slot.hpp>
#endif
