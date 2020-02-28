/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
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

	namespace
	{
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		std::pair<iterator, bool> dgram_match_role(iterator begin, iterator end)
		{
			iterator i = begin;
			while (i != end)
			{
				// If 0~254, current byte are the payload length.
				if (std::uint8_t(*i) < std::uint8_t(254))
				{
					std::uint8_t payload_size = std::uint8_t(*i);

					++i;

					if (end - i < int(payload_size))
						break;

					return std::pair(i + payload_size, true);
				}

				// If 254, the following 2 bytes interpreted as a 16-bit unsigned integer
				// (the most significant bit MUST be 0) are the payload length.
				if (std::uint8_t(*i) == std::uint8_t(254))
				{
					++i;

					if (end - i < 2)
						break;

					std::uint16_t payload_size = *(reinterpret_cast<const std::uint16_t*>(i.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(&payload_size));
					}

					i += 2;
					if (end - i < int(payload_size))
						break;

					return std::pair(i + payload_size, true);
				}

				// If 255, the following 8 bytes interpreted as a 64-bit unsigned integer
				// (the most significant bit MUST be 0) are the payload length.
				if (std::uint8_t(*i) == 255)
				{
					++i;

					if (end - i < 8)
						break;

					std::uint64_t payload_size = *(reinterpret_cast<const std::uint64_t*>(i.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint64_t)>(reinterpret_cast<std::uint8_t*>(&payload_size));
					}

					i += 8;
					if (std::uint64_t(end - i) < payload_size)
						break;

					return std::pair(i + payload_size, true);
				}

				ASIO2_ASSERT(false);
			}
			return std::pair(begin, false);
		}
	}
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
		inline auto& operator()() { return dgram_match_role; }
	protected:
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
