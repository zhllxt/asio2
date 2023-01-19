/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * boost/uuid
 * https://lowrey.me/guid-generation-in-c-11/
 * https://github.com/mariusbancila/stduuid
 * https://stackoverflow.com/questions/13445688/how-to-generate-a-random-number-in-c
 */

#ifndef __ASIO2_UUID_IMPL_HPP__
#define __ASIO2_UUID_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstring>
#include <cstdint>
#include <cstddef>

#include <string>
#include <random>
#include <limits>
#include <memory>
#include <array>
#include <algorithm>
#include <iterator>

namespace asio2
{
	/**
	 * UUID Generation in C++11
	 */
	class uuid
	{
	protected:
		struct random_enginer
		{
			std::seed_seq                                sseq_;
			std::mt19937                                 engine_;
			std::uniform_int_distribution<std::uint32_t> distribution_;

			template<class InputIt>
			explicit random_enginer(InputIt begin, InputIt end)
				: sseq_(begin, end)
				, engine_(sseq_)
			{
			}

			inline std::uint32_t get()
			{
				return distribution_(engine_);
			}
		};

	public:
		uuid()
		{
			std::random_device rd{};
			std::array<std::uint32_t, std::mt19937::state_size> sb{};
			std::generate(std::begin(sb), std::end(sb), std::ref(rd));

			this->random_enginer_ = std::make_unique<random_enginer>(std::begin(sb), std::end(sb));
		}
		~uuid() = default;

		uuid(const uuid&) = delete;
		uuid(uuid&&) noexcept = default;

		uuid& operator=(const uuid&) = delete;
		uuid& operator=(uuid&&) noexcept = default;

		inline uuid& operator()()
		{
			return generate();
		}

		inline uuid& next()
		{
			return generate();
		}

		inline uuid& generate()
		{
			for (std::size_t i = 0; i < sizeof(data); i += sizeof(std::uint32_t))
			{
				*reinterpret_cast<std::uint32_t*>(data + i) = this->random_enginer_->get();
			}

			// set variant
			// must be 0b10xxxxxx
			*(data + 8) &= 0xBF;
			*(data + 8) |= 0x80;

			// set version
			// must be 0b0100xxxx
			*(data + 6) &= 0x4F; //0b01001111
			*(data + 6) |= 0x40; //0b01000000

			return (*this);
		}

		/**
		 * @brief convert the uuid bytes to std::string
		 */
		inline std::string str(bool upper = false, bool group = true) noexcept
		{
			std::string result;

			if (group)
			{
				// 00000000-0000-0000-0000-000000000000

				result.reserve(36);

				for (std::size_t i = 0; i < sizeof(data); )
				{
					int n = static_cast<int>(result.size());

					if (n == 8 || n == 13 || n == 18 || n == 23)
					{
						result += '-';
						continue;
					}

					const int hi = ((*(data + i)) >> 4) & 0x0F;
					result += to_char(hi, upper);

					const int lo = (*(data + i)) & 0x0F;
					result += to_char(lo, upper);

					++i;
				}
			}
			else
			{
				// 00000000000000000000000000000000

				result.reserve(32);

				for (std::size_t i = 0; i < sizeof(data); ++i)
				{
					const int hi = ((*(data + i)) >> 4) & 0x0F;
					result += to_char(hi, upper);

					const int lo = (*(data + i)) & 0x0F;
					result += to_char(lo, upper);
				}
			}

			return result;
		}

		// This is equivalent to boost::hash_range(u.begin(), u.end());
		inline std::size_t hash() noexcept
		{
			std::size_t seed = 0;
			for (std::size_t i = 0; i < sizeof(data); ++i)
			{
				seed ^= static_cast<std::size_t>(*(data + i)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			return seed;
		}

		enum class variant_type
		{
			ncs,       // NCS backward compatibility
			rfc,       // defined in RFC 4122 document
			microsoft, // Microsoft Corporation backward compatibility
			future     // future definition
		};

		inline variant_type variant() const noexcept
		{
			// variant is stored in octet 7
			// which is index 8, since indexes count backwards
			unsigned char octet7 = data[8]; // octet 7 is array index 8

			if ((octet7 & 0x80) == 0x00)
			{
				// 0b0xxxxxxx
				return variant_type::ncs;
			}
			else if ((octet7 & 0xC0) == 0x80)
			{
				// 0b10xxxxxx
				return variant_type::rfc;
			}
			else if ((octet7 & 0xE0) == 0xC0)
			{
				// 0b110xxxxx
				return variant_type::microsoft;
			}
			else
			{
				//assert( (octet7 & 0xE0) == 0xE0 ) // 0b111xxxx
				return variant_type::future;
			}
		}

		enum class version_type
		{
			unknown             = 0, // only possible for nil or invalid uuids
			time_based          = 1, // The time-based version specified in RFC 4122
			dce_security        = 2, // DCE Security version, with embedded POSIX UIDs.
			name_based_md5      = 3, // The name-based version specified in RFS 4122 with MD5 hashing
			random_number_based = 4, // The randomly or pseudo-randomly generated version specified in RFS 4122
			name_based_sha1     = 5	 // The name-based version specified in RFS 4122 with SHA1 hashing
		};

		inline version_type version() const noexcept
		{
			// version is stored in octet 9
			// which is index 6, since indexes count backwards
			uint8_t octet9 = data[6];

			if ((octet9 & 0xF0) == 0x10)
			{
				return version_type::time_based;
			}
			else if ((octet9 & 0xF0) == 0x20)
			{
				return version_type::dce_security;
			}
			else if ((octet9 & 0xF0) == 0x30)
			{
				return version_type::name_based_md5;
			}
			else if ((octet9 & 0xF0) == 0x40)
			{
				return version_type::random_number_based;
			}
			else if ((octet9 & 0xF0) == 0x50)
			{
				return version_type::name_based_sha1;
			}
			else
			{
				return version_type::unknown;
			}
		}

		inline std::string short_uuid(int bytes)
		{
			// use base64 chars as the short uuid chars
			static std::string const base64_chars =
				"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789+/";

			std::string result;

			result.reserve(bytes);

			std::uniform_int_distribution<int> d(0, int(base64_chars.size() - 1));

			for (int i = 0; i < bytes; ++i)
			{
				result += base64_chars[d(this->random_enginer_->engine_)];
			}

			return result;
		}

		template <typename CharT>
		constexpr inline unsigned char hex2char(CharT const ch)
		{
			if (ch >= static_cast<CharT>('0') && ch <= static_cast<CharT>('9'))
				return static_cast<unsigned char>(ch - static_cast<CharT>('0'));
			if (ch >= static_cast<CharT>('a') && ch <= static_cast<CharT>('f'))
				return static_cast<unsigned char>(10 + ch - static_cast<CharT>('a'));
			if (ch >= static_cast<CharT>('A') && ch <= static_cast<CharT>('F'))
				return static_cast<unsigned char>(10 + ch - static_cast<CharT>('A'));
			return 0;
		}

		template <typename CharT>
		constexpr inline bool is_hex(CharT const ch)
		{
			return
				(ch >= static_cast<CharT>('0') && ch <= static_cast<CharT>('9')) ||
				(ch >= static_cast<CharT>('a') && ch <= static_cast<CharT>('f')) ||
				(ch >= static_cast<CharT>('A') && ch <= static_cast<CharT>('F'));
		}

	protected:
		inline char to_char(int i, bool upper) noexcept
		{
			if (i <= 9)
			{
				return static_cast<char>('0' + i);
			}
			else
			{
				return static_cast<char>((upper ? 'A' : 'a') + (i - 10));
			}
		}

	public:
		std::uint8_t data[16]{};

	protected:
		std::unique_ptr<random_enginer> random_enginer_;
	};
}

#endif // !__ASIO2_UUID_IMPL_HPP__
