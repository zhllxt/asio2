/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_CONDITION_WRAP_HPP__
#define __ASIO2_CONDITION_WRAP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <string_view>
#include <regex>

#include <asio2/base/selector.hpp>

namespace asio2::detail
{
	struct use_sync_t {};
	struct use_kcp_t {};
	struct use_dgram_t {};

	struct dgram_parse_recv_op
	{
		template<class DynamicBuffer, class Handler>
		std::size_t operator()(DynamicBuffer & buffer, Handler && handler)
		{
			std::size_t minimum = 1;
			while (buffer.size() > 0)
			{
				const unsigned char* payload_data = nullptr;
				std::size_t payload_size = 0;
				std::size_t consumed = 0, total = buffer.size();

				const unsigned char* p = static_cast<const unsigned char*>(buffer.data().data());
				switch (p[0])
				{
				case 254: // If 254, the following 2 bytes interpreted as a 16-bit unsigned integer (the most significant bit MUST be 0) are the payload length.
					if (total >= 1 + 2)
					{
						payload_size = *(reinterpret_cast<std::uint16_t*>(const_cast<unsigned char*>(p + 1)));
						if (total - 1 - 2 >= payload_size)
						{
							payload_data = p + 1 + 2;
							consumed = 1 + 2 + payload_size;
							minimum = 1;
						}
						else
							minimum = payload_size - (total - 1 - 2);
					}
					else
						minimum = 1 + 2 - total;
					break;
				case 255: // If 255, the following 8 bytes interpreted as a 64-bit unsigned integer (the most significant bit MUST be 0) are the payload length.
					if (total >= 1 + 8)
					{
						payload_size = std::size_t(*(reinterpret_cast<std::uint64_t*>(const_cast<unsigned char*>(p + 1))));
						if (total - 1 - 8 >= payload_size)
						{
							payload_data = p + 1 + 8;
							consumed = 1 + 8 + payload_size;
							minimum = 1;
						}
						else
							minimum = payload_size - (total - 1 - 8);
					}
					else
						minimum = 1 + 8 - total;
					break;
				default:  // If 0~254, current byte are the payload length.
					payload_size = p[0];
					if (total - 1 >= payload_size)
					{
						payload_data = p + 1;
						consumed = 1 + payload_size;
						minimum = 1;
					}
					else
						minimum = payload_size - (total - 1);
					break;
				}

				if (consumed == 0)
					break;

				handler(payload_data, payload_size);

				buffer.consume(consumed);
			}
			return minimum;
		}
	};

	struct dgram_send_head_op
	{
		template<class DynamicBuffer, class Stream>
		std::size_t operator()(Stream & stream, DynamicBuffer & buffer, error_code & ec)
		{
			std::size_t sent_bytes = 0;
			if (buffer.size() > (std::numeric_limits<std::uint16_t>::max)())
			{
				std::uint8_t head[9];
				head[0] = static_cast<std::uint8_t>(255);
				std::uint64_t size = buffer.size();
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint64_t));
				sent_bytes = asio::write(stream, asio::buffer(head), ec);
			}
			else if (buffer.size() > 253)
			{
				std::uint8_t head[3];
				head[0] = static_cast<std::uint8_t>(254);
				std::uint16_t size = static_cast<std::uint16_t>(buffer.size());
				std::memcpy(&head[1], reinterpret_cast<const void*>(&size), sizeof(std::uint16_t));
				sent_bytes = asio::write(stream, asio::buffer(head), ec);
			}
			else
			{
				std::uint8_t head[1];
				head[0] = static_cast<std::uint8_t>(buffer.size());
				sent_bytes = asio::write(stream, asio::buffer(head), ec);
			}
			return sent_bytes;
		}
	};
}

namespace asio2::detail
{
	template<class T>
	class condition_wrap
	{
	public:
		using type = T;
		condition_wrap(T c) : condition_(std::move(c)) {}
		inline T & operator()() { return this->condition_; }
	protected:
		T condition_;
	};

	template<>
	class condition_wrap<void>
	{
	public:
		using type = void;
		condition_wrap() = default;
	};

	template<>
	class condition_wrap<char>
	{
	public:
		using type = char;
		condition_wrap(char c) : condition_(c) {}
		inline char operator()() { return this->condition_; }
	protected:
		char condition_ = '\0';
	};

	//template<>
	//class condition_wrap<std::string>
	//{
	//public:
	//	using type = std::string;
	//	condition_wrap(const std::string & c) : condition_(c) {}
	//	condition_wrap(std::string && c) : condition_(std::move(c)) {}
	//protected:
	//	std::string condition_;
	//};

	//template<>
	//class condition_wrap<std::string_view>
	//{
	//public:
	//	using type = std::string_view;
	//	condition_wrap(std::string_view c) : condition_(c) {}
	//protected:
	//	std::string_view condition_;
	//};

	//template<>
	//class condition_wrap<std::regex>
	//{
	//public:
	//	using type = std::regex;
	//	condition_wrap(const std::regex & c) : condition_(c) {}
	//	condition_wrap(std::regex && c) : condition_(std::move(c)) {}
	//protected:
	//	std::regex condition_;
	//};

	template<>
	class condition_wrap<use_dgram_t>
	{
	public:
		using type = use_dgram_t;
		condition_wrap(use_dgram_t) {}
		inline asio::detail::transfer_at_least_t operator()() { return asio::transfer_at_least(this->minimum_); }
		inline void need(std::size_t size) { this->minimum_ = size; }
	protected:
		std::size_t minimum_ = 1;
	};

	template<>
	class condition_wrap<use_kcp_t>
	{
	public:
		using type = use_kcp_t;
		condition_wrap(use_kcp_t) {}
		inline asio::detail::transfer_at_least_t operator()() { return asio::transfer_at_least(1); }
	protected:
	};
}

namespace asio2
{
	constexpr static detail::use_dgram_t use_dgram;

	constexpr static detail::use_sync_t  use_sync;

	// https://github.com/skywind3000/kcp
	constexpr static detail::use_kcp_t   use_kcp;
}

#endif // !__ASIO2_CONDITION_WRAP_HPP__
