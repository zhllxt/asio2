/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * refrenced from : /beast/core/detect_ssl.hpp
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * Note : this functionality not yet implemented
 *
 */

#ifndef __ASIO2_MQTT_DETECT_WEBSOCKET_HPP__
#define __ASIO2_MQTT_DETECT_WEBSOCKET_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <type_traits>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#ifdef ASIO_STANDALONE
	#include <asio/yield.hpp>
#else
	#include <boost/asio/yield.hpp>
#endif

namespace asio2::detail
{
	template<class DetectHandler, class AsyncReadStream, class DynamicBuffer>
	class detect_websocket_op : public asio::coroutine
	{
		DetectHandler handler_;

		AsyncReadStream& stream_;

		// The callers buffer is used to hold all received data
		DynamicBuffer& buffer_;

		// We're going to need this in case we have to post the handler
		error_code ec_;

		beast::tribool result_ = false;

	public:
		// Completion handlers must be MoveConstructible.
		detect_websocket_op(detect_websocket_op&&) = default;

		// Construct the operation. The handler is deduced through
		// the template type `DetectHandler`, this lets the same constructor
		// work properly for both lvalues and rvalues.
		//
		detect_websocket_op(
			DetectHandler&& handler,
			AsyncReadStream& stream,
			DynamicBuffer& buffer)
			: handler_(std::forward<DetectHandler>(handler))
			, stream_(stream)
			, buffer_(buffer)
		{
			// This starts the operation. We pass `false` to tell the
			// algorithm that it needs to use asio::post if it wants to
			// complete immediately. This is required by Networking,
			// as initiating functions are not allowed to invoke the
			// completion handler on the caller's thread before
			// returning.
			(*this)({}, 0, false);
		}

		// Our main entry point. This will get called as our
		// intermediate operations complete. Definition below.
		//
		// The parameter `cont` indicates if we are being called subsequently
		// from the original invocation
		//
		void operator()(error_code ec, std::size_t bytes_transferred, bool cont = true)
		{
			// This introduces the scope of the stackless coroutine
			reenter(*this)
			{
				// Loop until an error occurs or we get a definitive answer
				for(;;)
				{
					// There could already be a hello in the buffer so check first
					result_ = is_websocket_upgrade_request(buffer_.data());

					// If we got an answer, then the operation is complete
					if(! beast::indeterminate(result_))
						break;

					// Try to fill our buffer by reading from the stream.
					// The function read_size calculates a reasonable size for the
					// amount to read next, using existing capacity if possible to
					// avoid allocating memory, up to the limit of 1536 bytes which
					// is the size of a normal TCP frame.
					//
					// `async_read_some` expects a ReadHandler as the completion
					// handler. The signature of a read handler is void(error_code, size_t),
					// and this function matches that signature (the `cont` parameter has
					// a default of true). We pass `std::move(*this)` as the completion
					// handler for the read operation. This transfers ownership of this
					// entire state machine back into the `async_read_some` operation.
					// Care must be taken with this idiom, to ensure that parameters
					// passed to the initiating function which could be invalidated
					// by the move, are first moved to the stack before calling the
					// initiating function.

					yield stream_.async_read_some(buffer_.prepare(
						read_size(buffer_, 1536)), std::move(*this));

					// Commit what we read into the buffer's input area.
					buffer_.commit(bytes_transferred);

					// Check for an error
					if(ec)
						break;
				}

				// If `cont` is true, the handler will be invoked directly.
				//
				// Otherwise, the handler cannot be invoked directly, because
				// initiating functions are not allowed to call the handler
				// before returning. Instead, the handler must be posted to
				// the I/O context. We issue a zero-byte read using the same
				// type of buffers used in the ordinary read above, to prevent
				// the compiler from creating an extra instantiation of the
				// function template. This reduces compile times and the size
				// of the program executable.

				if(! cont)
				{
					// Save the error, otherwise it will be overwritten with
					// a successful error code when this read completes
					// immediately.
					ec_ = ec;

					// Zero-byte reads and writes are guaranteed to complete
					// immediately with succcess. The type of buffers and the
					// type of handler passed here need to exactly match the types
					// used in the call to async_read_some above, to avoid
					// instantiating another version of the function template.

					yield stream_.async_read_some(buffer_.prepare(0), std::move(*this));

					// Restore the saved error code
					ec = ec_;
				}

				// Invoke the final handler.
				// At this point, we are guaranteed that the original initiating
				// function is no longer on our stack frame.

				this->handler_(ec, static_cast<bool>(result_));
			}
		}
	};


	struct run_detect_websocket_op
	{
		template<class DetectHandler, class AsyncReadStream, class DynamicBuffer>
		void operator()(DetectHandler&& h, AsyncReadStream* s, DynamicBuffer& b)
		{
			detect_websocket_op<DetectHandler, AsyncReadStream, DynamicBuffer>(
				std::forward<DetectHandler>(h), *s, b);
		}
	};


	template <class ConstBufferSequence>
	beast::tribool is_websocket_upgrade_request(ConstBufferSequence const& buffers)
	{
		// Make sure buffers meets the requirements
		static_assert(
			asio::is_const_buffer_sequence<ConstBufferSequence>::value,
			"ConstBufferSequence type requirements not met");

		// Flatten the input buffers into a single contiguous range
		// of bytes on the stack to make it easier to work with the data.
		unsigned char buf[9];
		auto const n = asio::buffer_copy(
			asio::mutable_buffer(buf, sizeof(buf)), buffers);

		// Can't do much without any bytes
		if(n < 1)
			return beast::indeterminate;

		// Require the first byte to be 0x16, indicating a TLS handshake record
		if(buf[0] != 0x16)
			return false;

		// We need at least 5 bytes to know the record payload size
		if(n < 5)
			return beast::indeterminate;

		// Calculate the record payload size
		std::uint32_t const length = (buf[3] << 8) + buf[4];

		// A ClientHello message payload is at least 34 bytes.
		// There can be multiple handshake messages in the same record.
		if(length < 34)
			return false;

		// We need at least 6 bytes to know the handshake type
		if(n < 6)
			return beast::indeterminate;

		// The handshake_type must be 0x01 == client_hello
		if(buf[5] != 0x01)
			return false;

		// We need at least 9 bytes to know the payload size
		if(n < 9)
			return beast::indeterminate;

		// Calculate the message payload size
		std::uint32_t const size =
			(buf[6] << 16) + (buf[7] << 8) + buf[8];

		// The message payload can't be bigger than the enclosing record
		if(size + 4 > length)
			return false;

		// This can only be a TLS client_hello message
		return true;
	}


	template<class SyncReadStream, class DynamicBuffer>
	bool detect_websocket(SyncReadStream& stream, DynamicBuffer& buffer, error_code& ec)
	{
		// Make sure arguments meet the requirements

		static_assert(
			is_sync_read_stream<SyncReadStream>::value,
			"SyncReadStream type requirements not met");
    
		static_assert(
			asio::is_dynamic_buffer<DynamicBuffer>::value,
			"DynamicBuffer type requirements not met");

		// Loop until an error occurs or we get a definitive answer
		for(;;)
		{
			// There could already be data in the buffer
			// so we do this first, before reading from the stream.
			auto const result = detail::is_websocket_upgrade_request(buffer.data());

			// If we got an answer, return it
			if(! beast::indeterminate(result))
			{
				// A definite answer is a success
				ec = {};
				return static_cast<bool>(result);
			}

			// Try to fill our buffer by reading from the stream.
			// The function read_size calculates a reasonable size for the
			// amount to read next, using existing capacity if possible to
			// avoid allocating memory, up to the limit of 1536 bytes which
			// is the size of a normal TCP frame.

			std::size_t const bytes_transferred = stream.read_some(
				buffer.prepare(beast::read_size(buffer, 1536)), ec);

			// Commit what we read into the buffer's input area.
			buffer.commit(bytes_transferred);

			// Check for an error
			if(ec)
				break;
		}

		// error
		return false;
	}


	// Here is the implementation of the asynchronous initiation function
	template<
		class AsyncReadStream,
		class DynamicBuffer,
		class CompletionToken = asio::default_completion_token_t<beast::executor_type<AsyncReadStream>>
	>
	auto async_detect_websocket(
		AsyncReadStream& stream,
		DynamicBuffer& buffer,
		CompletionToken&& token = asio::default_completion_token_t<beast::executor_type<AsyncReadStream>>{}) ->
			typename asio::async_result< /*< `async_result` customizes the return value based on the completion token >*/
				typename std::decay<CompletionToken>::type,
				void(error_code, bool)>::return_type /*< This is the signature for the completion handler >*/
	{
		// Make sure arguments meet the type requirements

		static_assert(
			is_async_read_stream<AsyncReadStream>::value,
			"SyncReadStream type requirements not met");

		static_assert(
			asio::is_dynamic_buffer<DynamicBuffer>::value,
			"DynamicBuffer type requirements not met");

		// The function `asio::async_initate` uses customization points
		// to allow one asynchronous initiating function to work with
		// all sorts of notification systems, such as callbacks but also
		// fibers, futures, coroutines, and user-defined types.
		//
		// It works by capturing all of the arguments using perfect
		// forwarding, and then depending on the specialization of
		// `asio::async_result` for the type of `CompletionToken`,
		// the `initiation` object will be invoked with the saved
		// parameters and the actual completion handler. Our
		// initiating object is `run_detect_websocket_op`.
		//
		// Non-const references need to be passed as pointers,
		// since we don't want a decay-copy.

		return asio::async_initiate<CompletionToken, void(error_code, bool)>(
			detail::run_detect_websocket_op{},
			std::forward<CompletionToken>(token),
			&stream, // pass the reference by pointer
			buffer);
	}
}

#ifdef ASIO_STANDALONE
	#include <asio/unyield.hpp>
#else
	#include <boost/asio/unyield.hpp>
#endif

#endif // !__ASIO2_MQTT_DETECT_WEBSOCKET_HPP__
