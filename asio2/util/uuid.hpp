/*
 * COPYRIGHT (C) 2019-2020, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * code come from : boost/uuid
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

namespace asio2
{
	/**
	 * UUID Generation in C++11
	 * Reference from the boost library
	 */
	class uuid
	{
	public:
		uuid()
		{
			unsigned long random_value = random();
			for (int i = 0, it = 0; it < sizeof(data); ++it, ++i)
			{
				if (i == sizeof(unsigned long))
				{
					random_value = random();
					i = 0;
				}

				// static_cast gets rid of warnings of converting unsigned long to std::uint8_t
				data[it] = static_cast<std::uint8_t>((random_value >> (i * 8)) & 0xFF);
			}

			// set variant
			// must be 0b10xxxxxx
			*(data + 8) &= 0xBF;
			*(data + 8) |= 0x80;

			// set version
			// must be 0b0100xxxx
			*(data + 6) &= 0x4F; //0b01001111
			*(data + 6) |= 0x40; //0b01000000
		}
		~uuid() = default;

		uuid(const uuid&) = default;
		uuid(uuid&&) = default;

		uuid& operator=(const uuid&) = default;
		uuid& operator=(uuid&&) = default;

		inline std::string str(bool upper = false) noexcept
		{
			std::string result;
			result.reserve(32);

			for (int it = 0; it < sizeof(data); ++it)
			{
				const int hi = ((*(data + it)) >> 4) & 0x0F;
				result += to_char(hi, upper);

				const int lo = (*(data + it)) & 0x0F;
				result += to_char(lo, upper);
			}
			return result;
		}

		// This is equivalent to boost::hash_range(u.begin(), u.end());
		inline std::size_t hash() noexcept
		{
			std::size_t seed = 0;
			for (int it = 0; it < sizeof(data); ++it)
			{
				seed ^= static_cast<std::size_t>(*(data + it)) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			return seed;
		}

		enum variant_type
		{
			variant_ncs, // NCS backward compatibility
			variant_rfc_4122, // defined in RFC 4122 document
			variant_microsoft, // Microsoft Corporation backward compatibility
			variant_future // future definition
		};
		variant_type variant() const noexcept
		{
			// variant is stored in octet 7
			// which is index 8, since indexes count backwards
			unsigned char octet7 = data[8]; // octet 7 is array index 8
			if ((octet7 & 0x80) == 0x00) { // 0b0xxxxxxx
				return variant_ncs;
			}
			else if ((octet7 & 0xC0) == 0x80) { // 0b10xxxxxx
				return variant_rfc_4122;
			}
			else if ((octet7 & 0xE0) == 0xC0) { // 0b110xxxxx
				return variant_microsoft;
			}
			else {
				//assert( (octet7 & 0xE0) == 0xE0 ) // 0b111xxxx
				return variant_future;
			}
		}

		enum version_type
		{
			version_unknown = -1,
			version_time_based = 1,
			version_dce_security = 2,
			version_name_based_md5 = 3,
			version_random_number_based = 4,
			version_name_based_sha1 = 5
		};
		version_type version() const noexcept
		{
			// version is stored in octet 9
			// which is index 6, since indexes count backwards
			uint8_t octet9 = data[6];
			if ((octet9 & 0xF0) == 0x10) {
				return version_time_based;
			}
			else if ((octet9 & 0xF0) == 0x20) {
				return version_dce_security;
			}
			else if ((octet9 & 0xF0) == 0x30) {
				return version_name_based_md5;
			}
			else if ((octet9 & 0xF0) == 0x40) {
				return version_random_number_based;
			}
			else if ((octet9 & 0xF0) == 0x50) {
				return version_name_based_sha1;
			}
			else {
				return version_unknown;
			}
		}

		static inline std::string short_uuid(int bytes)
		{
			char keys[]
			{
				'+', '-',
				'0', '1', '2', '3', '4', '5', '6', '7', '8','9',
				'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
				'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
			};

			std::string s;
			s.reserve(bytes);

			for (int it = 0; it < bytes; ++it)
			{
				s += keys[random(0, sizeof(keys) - 1)];
			}

			return s;
		}

		static inline unsigned long random(
			unsigned long mini = (std::numeric_limits<unsigned long>::min)(),
			unsigned long maxi = (std::numeric_limits<unsigned long>::max)())
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<unsigned long> dis(mini, maxi);
			return dis(gen);
		}

	protected:
		inline char to_char(int i, bool upper)
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
		std::uint8_t data[16];
	};
}

#endif // !__ASIO2_UUID_IMPL_HPP__
