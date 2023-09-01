/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * code come from : boost/uuid/detail/md5.hpp
 */

#ifndef __ASIO2_MD5_IMPL_HPP__
#define __ASIO2_MD5_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstring>
#include <cstdint>
#include <cstddef>

#include <string>
#include <fstream>

#include <asio2/config.hpp>
#include <asio2/base/detail/filesystem.hpp>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/predef/other/endian.h>)
#include <boost/predef/other/endian.h>
#ifndef ASIO2_ENDIAN_LITTLE_BYTE
#define ASIO2_ENDIAN_LITTLE_BYTE BOOST_ENDIAN_LITTLE_BYTE
#endif
#else
#include <asio2/bho/predef/other/endian.h>
#ifndef ASIO2_ENDIAN_LITTLE_BYTE
#define ASIO2_ENDIAN_LITTLE_BYTE BHO_ENDIAN_LITTLE_BYTE
#endif
#endif

#ifndef ASIO2_DISABLE_AUTO_BOOST_UUID_COMPAT_PRE_1_71_MD5
#ifndef BOOST_UUID_COMPAT_PRE_1_71_MD5
#define BOOST_UUID_COMPAT_PRE_1_71_MD5
#endif // !BOOST_UUID_COMPAT_PRE_1_71_MD5
#endif // !ASIO2_DISABLE_AUTO_BOOST_UUID_COMPAT_PRE_1_71_MD5

namespace asio2
{
	class md5
	{
	public:
		md5()
		{
			MD5_Init(&ctx_);
		}

		/**
		 * @construct Construct a MD5 object with a std::string.
		 */
		md5(const std::string & message) : md5()
		{
			MD5_Update(&ctx_, (const void*)message.data(),
				static_cast<unsigned long>(message.length()));
		}

		/**
		 * @construct Construct a MD5 object with a char pointer.
		 */
		md5(const char * message) : md5()
		{
			MD5_Update(&ctx_, (const void*)message,
				static_cast<unsigned long>(std::strlen(message)));
		}

		/**
		 * @construct Construct a MD5 object with a unsigned char pointer.
		 */
		md5(const void * message, std::size_t size) : md5()
		{
			MD5_Update(&ctx_, message, static_cast<unsigned long>(size));
		}

		/**
		 * @construct Construct a MD5 object from a file path.
		 */
		md5(const std::filesystem::path& filepath) : md5()
		{
			std::error_code ec{};
			std::uintmax_t size = std::filesystem::file_size(filepath, ec);
			if (!ec && size > static_cast<std::uintmax_t>(0))
			{
				std::fstream file(filepath, std::ios::in | std::ios::binary);
				if (file)
				{
					char buffer[1024];

					while (size > static_cast<std::uintmax_t>(0))
					{
						if (size >= static_cast<std::uintmax_t>(1024))
						{
							if (!file.read(buffer, 1024))
								break;

							MD5_Update(&ctx_, (const void*)buffer, static_cast<unsigned long>(1024));

							size -= 1024;
						}
						else
						{
							if (!file.read(buffer, static_cast<std::streamsize>(size)))
								break;

							MD5_Update(&ctx_, (const void*)buffer, static_cast<unsigned long>(size));

							size -= size;
						}
					}
				}
			}
		}

		/* Convert digest to std::string value */
		std::string str(bool upper = false)
		{
			/* Hex numbers. */
			char hex_upper[16] = {
				'0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'A', 'B',
				'C', 'D', 'E', 'F'
			};
			char hex_lower[16] = {
				'0', '1', '2', '3',
				'4', '5', '6', '7',
				'8', '9', 'a', 'b',
				'c', 'd', 'e', 'f'
			};

			std::uint8_t digest[16];
			get_digest(digest);
			std::string str;
			str.reserve(16 << 1);
			for (std::size_t i = 0; i < 16; ++i)
			{
				int t = digest[i];
				int a = t / 16;
				int b = t % 16;
				str.append(1, upper ? hex_upper[a] : hex_lower[a]);
				str.append(1, upper ? hex_upper[b] : hex_lower[b]);
			}
			return str;
		}

		void process_byte(unsigned char byte)
		{
			MD5_Update(&ctx_, &byte, 1);
		}

		void process_bytes(void const* buffer, std::size_t byte_count)
		{
			MD5_Update(&ctx_, buffer, static_cast<unsigned long>(byte_count));
		}

		void get_digest(std::uint8_t digest[16])
		{
			MD5_Final(reinterpret_cast<unsigned char *>(&digest[0]), &ctx_);
		}

		unsigned char get_version() const
		{
			// RFC 4122 Section 4.1.3
			//return uuid::version_name_based_md5;
			return static_cast<unsigned char>(3);
		}

	private:

		/* Any 32-bit or wider unsigned integer data type will do */
		typedef uint32_t MD5_u32plus;

		typedef struct {
			MD5_u32plus lo, hi;
			MD5_u32plus a, b, c, d;
			unsigned char buffer[64];
			MD5_u32plus block[16];
		} MD5_CTX;

		/*
		 * The basic MD5 functions.
		 *
		 * F and G are optimized compared to their RFC 1321 definitions for
		 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
		 * implementation.
		 */
		inline MD5_u32plus ASIO2_UUID_DETAIL_MD5_F(MD5_u32plus x, MD5_u32plus y, MD5_u32plus z) { return ((z) ^ ((x) & ((y) ^ (z)))); }
		inline MD5_u32plus ASIO2_UUID_DETAIL_MD5_G(MD5_u32plus x, MD5_u32plus y, MD5_u32plus z) { return ((y) ^ ((z) & ((x) ^ (y)))); }
		inline MD5_u32plus ASIO2_UUID_DETAIL_MD5_H(MD5_u32plus x, MD5_u32plus y, MD5_u32plus z) { return (((x) ^ (y)) ^ (z)); }
		inline MD5_u32plus ASIO2_UUID_DETAIL_MD5_H2(MD5_u32plus x, MD5_u32plus y, MD5_u32plus z) { return ((x) ^ ((y) ^ (z))); }
		inline MD5_u32plus ASIO2_UUID_DETAIL_MD5_I(MD5_u32plus x, MD5_u32plus y, MD5_u32plus z) { return ((y) ^ ((x) | ~(z))); }

		/*
		 * The MD5 transformation for all four rounds.
		 */
#define ASIO2_UUID_DETAIL_MD5_STEP(f, a, b, c, d, x, t, s) \
        (a) += f((b), (c), (d)) + (x) + (t); \
        (a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
        (a) += (b);

		 /*
		  * SET reads 4 input bytes in little-endian byte order and stores them in a
		  * properly aligned word in host byte order.
		  *
		  * The check for little-endian architectures that tolerate unaligned memory
		  * accesses is just an optimization.  Nothing will break if it fails to detect
		  * a suitable architecture.
		  *
		  * Unfortunately, this optimization may be a C strict aliasing rules violation
		  * if the caller's data buffer has effective type that cannot be aliased by
		  * MD5_u32plus.  In practice, this problem may occur if these MD5 routines are
		  * inlined into a calling function, or with future and dangerously advanced
		  * link-time optimizations.  For the time being, keeping these MD5 routines in
		  * their own translation unit avoids the problem.
		  */
#if defined(__i386__) || defined(__x86_64__) || defined(__vax__)
#define ASIO2_UUID_DETAIL_MD5_SET(n) \
        (*(MD5_u32plus *)&ptr[(n) * 4])
#define ASIO2_UUID_DETAIL_MD5_GET(n) \
        ASIO2_UUID_DETAIL_MD5_SET(n)
#else
#define ASIO2_UUID_DETAIL_MD5_SET(n) \
        (ctx->block[(n)] = \
        (MD5_u32plus)ptr[(n) * 4] | \
        ((MD5_u32plus)ptr[(n) * 4 + 1] << 8) | \
        ((MD5_u32plus)ptr[(n) * 4 + 2] << 16) | \
        ((MD5_u32plus)ptr[(n) * 4 + 3] << 24))
#define ASIO2_UUID_DETAIL_MD5_GET(n) \
        (ctx->block[(n)])
#endif

		  /*
		   * This processes one or more 64-byte data blocks, but does NOT update the bit
		   * counters.  There are no alignment requirements.
		   */
		const void *body(MD5_CTX *ctx, const void *data, unsigned long size)
		{
			const unsigned char *ptr;
			MD5_u32plus a, b, c, d;
			MD5_u32plus saved_a, saved_b, saved_c, saved_d;

			ptr = (const unsigned char *)data;

			a = ctx->a;
			b = ctx->b;
			c = ctx->c;
			d = ctx->d;

			do {
				saved_a = a;
				saved_b = b;
				saved_c = c;
				saved_d = d;

				/* Round 1 */
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, a, b, c, d, ASIO2_UUID_DETAIL_MD5_SET(0), 0xd76aa478, 7)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, d, a, b, c, ASIO2_UUID_DETAIL_MD5_SET(1), 0xe8c7b756, 12)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, c, d, a, b, ASIO2_UUID_DETAIL_MD5_SET(2), 0x242070db, 17)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, b, c, d, a, ASIO2_UUID_DETAIL_MD5_SET(3), 0xc1bdceee, 22)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, a, b, c, d, ASIO2_UUID_DETAIL_MD5_SET(4), 0xf57c0faf, 7)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, d, a, b, c, ASIO2_UUID_DETAIL_MD5_SET(5), 0x4787c62a, 12)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, c, d, a, b, ASIO2_UUID_DETAIL_MD5_SET(6), 0xa8304613, 17)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, b, c, d, a, ASIO2_UUID_DETAIL_MD5_SET(7), 0xfd469501, 22)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, a, b, c, d, ASIO2_UUID_DETAIL_MD5_SET(8), 0x698098d8, 7)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, d, a, b, c, ASIO2_UUID_DETAIL_MD5_SET(9), 0x8b44f7af, 12)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, c, d, a, b, ASIO2_UUID_DETAIL_MD5_SET(10), 0xffff5bb1, 17)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, b, c, d, a, ASIO2_UUID_DETAIL_MD5_SET(11), 0x895cd7be, 22)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, a, b, c, d, ASIO2_UUID_DETAIL_MD5_SET(12), 0x6b901122, 7)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, d, a, b, c, ASIO2_UUID_DETAIL_MD5_SET(13), 0xfd987193, 12)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, c, d, a, b, ASIO2_UUID_DETAIL_MD5_SET(14), 0xa679438e, 17)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_F, b, c, d, a, ASIO2_UUID_DETAIL_MD5_SET(15), 0x49b40821, 22)

				/* Round 2 */
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(1), 0xf61e2562, 5)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(6), 0xc040b340, 9)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(11), 0x265e5a51, 14)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(0), 0xe9b6c7aa, 20)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(5), 0xd62f105d, 5)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(10), 0x02441453, 9)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(15), 0xd8a1e681, 14)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(4), 0xe7d3fbc8, 20)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(9), 0x21e1cde6, 5)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(14), 0xc33707d6, 9)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(3), 0xf4d50d87, 14)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(8), 0x455a14ed, 20)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(13), 0xa9e3e905, 5)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(2), 0xfcefa3f8, 9)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(7), 0x676f02d9, 14)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_G, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(12), 0x8d2a4c8a, 20)

				/* Round 3 */
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(5), 0xfffa3942, 4)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(8), 0x8771f681, 11)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(11), 0x6d9d6122, 16)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(14), 0xfde5380c, 23)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(1), 0xa4beea44, 4)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(4), 0x4bdecfa9, 11)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(7), 0xf6bb4b60, 16)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(10), 0xbebfbc70, 23)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(13), 0x289b7ec6, 4)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(0), 0xeaa127fa, 11)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(3), 0xd4ef3085, 16)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(6), 0x04881d05, 23)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(9), 0xd9d4d039, 4)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(12), 0xe6db99e5, 11)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(15), 0x1fa27cf8, 16)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_H2, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(2), 0xc4ac5665, 23)

				/* Round 4 */
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(0), 0xf4292244, 6)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(7), 0x432aff97, 10)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(14), 0xab9423a7, 15)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(5), 0xfc93a039, 21)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(12), 0x655b59c3, 6)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(3), 0x8f0ccc92, 10)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(10), 0xffeff47d, 15)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(1), 0x85845dd1, 21)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(8), 0x6fa87e4f, 6)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(15), 0xfe2ce6e0, 10)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(6), 0xa3014314, 15)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(13), 0x4e0811a1, 21)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, a, b, c, d, ASIO2_UUID_DETAIL_MD5_GET(4), 0xf7537e82, 6)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, d, a, b, c, ASIO2_UUID_DETAIL_MD5_GET(11), 0xbd3af235, 10)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, c, d, a, b, ASIO2_UUID_DETAIL_MD5_GET(2), 0x2ad7d2bb, 15)
				ASIO2_UUID_DETAIL_MD5_STEP(ASIO2_UUID_DETAIL_MD5_I, b, c, d, a, ASIO2_UUID_DETAIL_MD5_GET(9), 0xeb86d391, 21)

				a += saved_a;
				b += saved_b;
				c += saved_c;
				d += saved_d;

				ptr += 64;
			} while (size -= 64);

			ctx->a = a;
			ctx->b = b;
			ctx->c = c;
			ctx->d = d;

			return ptr;
		}

		void MD5_Init(MD5_CTX *ctx)
		{
			ctx->a = 0x67452301;
			ctx->b = 0xefcdab89;
			ctx->c = 0x98badcfe;
			ctx->d = 0x10325476;

			ctx->lo = 0;
			ctx->hi = 0;
		}

		void MD5_Update(MD5_CTX *ctx, const void *data, unsigned long size)
		{
			MD5_u32plus saved_lo;
			unsigned long used, available;

			saved_lo = ctx->lo;
			if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
				ctx->hi++;
			ctx->hi += size >> 29;

			used = saved_lo & 0x3f;

			if (used) {
				available = 64 - used;

				if (size < available) {
					memcpy(&ctx->buffer[used], data, size);
					return;
				}

				memcpy(&ctx->buffer[used], data, available);
				data = (const unsigned char *)data + available;
				size -= available;
				body(ctx, ctx->buffer, 64);
			}

			if (size >= 64) {
				data = body(ctx, data, size & ~(unsigned long)0x3f);
				size &= 0x3f;
			}

			memcpy(ctx->buffer, data, size);
		}

// This must remain consistent no matter the endianness
#define ASIO2_UUID_DETAIL_MD5_OUT(dst, src) \
        (dst)[0] = (unsigned char)(src); \
        (dst)[1] = (unsigned char)((src) >> 8); \
        (dst)[2] = (unsigned char)((src) >> 16); \
        (dst)[3] = (unsigned char)((src) >> 24);

	//
	// A big-endian issue with MD5 results was resolved
	// in boost 1.71.  If you generated md5 name-based uuids
	// with boost 1.66 through 1.70 and stored them, then
	// set the following compatibility flag to ensure that
	// your hash generation remains consistent.
	//
#if defined(BOOST_UUID_COMPAT_PRE_1_71_MD5) || defined(BHO_UUID_COMPAT_PRE_1_71_MD5)
	#define ASIO2_UUID_DETAIL_MD5_BYTE_OUT(dst, src) \
		ASIO2_UUID_DETAIL_MD5_OUT(dst, src)
#else
	//
	// We're copying into a byte buffer which is actually
	// backed by an unsigned int array, which later on
	// is then swabbed one more time by the basic name
	// generator.  Therefore the logic here is reversed.
	// This was done to minimize the impact to existing
	// name-based hash generation.  The correct fix would
	// be to make this and name generation endian-correct
	// but that would even break previously generated sha1
	// hashes too.
	//
#if ASIO2_ENDIAN_LITTLE_BYTE
	#define ASIO2_UUID_DETAIL_MD5_BYTE_OUT(dst, src) \
		(dst)[0] = (unsigned char)((src) >> 24); \
		(dst)[1] = (unsigned char)((src) >> 16); \
		(dst)[2] = (unsigned char)((src) >> 8); \
		(dst)[3] = (unsigned char)(src);
#else
	#define ASIO2_UUID_DETAIL_MD5_BYTE_OUT(dst, src) \
		(dst)[0] = (unsigned char)(src); \
		(dst)[1] = (unsigned char)((src) >> 8); \
		(dst)[2] = (unsigned char)((src) >> 16); \
		(dst)[3] = (unsigned char)((src) >> 24);
#endif
#endif // BOOST_UUID_COMPAT_PRE_1_71_MD5

		void MD5_Final(unsigned char *result, MD5_CTX *ctx)
		{
			unsigned long used, available;

			used = ctx->lo & 0x3f;

			ctx->buffer[used++] = 0x80;

			available = 64 - used;

			if (available < 8) {
				memset(&ctx->buffer[used], 0, available);
				body(ctx, ctx->buffer, 64);
				used = 0;
				available = 64;
			}

			memset(&ctx->buffer[used], 0, available - 8);

			ctx->lo <<= 3;
			ASIO2_UUID_DETAIL_MD5_OUT(&ctx->buffer[56], ctx->lo)
			ASIO2_UUID_DETAIL_MD5_OUT(&ctx->buffer[60], ctx->hi)

			body(ctx, ctx->buffer, 64);

			ASIO2_UUID_DETAIL_MD5_BYTE_OUT(&result[0], ctx->a)
			ASIO2_UUID_DETAIL_MD5_BYTE_OUT(&result[4], ctx->b)
			ASIO2_UUID_DETAIL_MD5_BYTE_OUT(&result[8], ctx->c)
			ASIO2_UUID_DETAIL_MD5_BYTE_OUT(&result[12], ctx->d)

			memset(ctx, 0, sizeof(*ctx));
		}

#undef ASIO2_UUID_DETAIL_MD5_OUT
#undef ASIO2_UUID_DETAIL_MD5_SET
#undef ASIO2_UUID_DETAIL_MD5_GET
#undef ASIO2_UUID_DETAIL_MD5_STEP
#undef ASIO2_UUID_DETAIL_MD5_BYTE_OUT

		MD5_CTX ctx_;
	};
}

#endif // !__ASIO2_MD5_IMPL_HPP__
