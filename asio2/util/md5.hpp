/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 *
 * code come from : @github https://github.com/JieweiWei
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

	/* Parameters of MD5. */
	#define MD5_S11 7
	#define MD5_S12 12
	#define MD5_S13 17
	#define MD5_S14 22
	#define MD5_S21 5
	#define MD5_S22 9
	#define MD5_S23 14
	#define MD5_S24 20
	#define MD5_S31 4
	#define MD5_S32 11
	#define MD5_S33 16
	#define MD5_S34 23
	#define MD5_S41 6
	#define MD5_S42 10
	#define MD5_S43 15
	#define MD5_S44 21

	/**
	 * @Basic MD5 functions.
	 *
	 * @param there unsigned int.
	 *
	 * @return one unsigned int.
	 */
	#define MD5_F(x, y, z) (((x) & (y)) | ((~x) & (z)))
	#define MD5_G(x, y, z) (((x) & (z)) | ((y) & (~z)))
	#define MD5_H(x, y, z) ((x) ^ (y) ^ (z))
	#define MD5_I(x, y, z) ((y) ^ ((x) | (~z)))

	/**
	 * @Rotate Left.
	 *
	 * @param {num} the raw number.
	 *
	 * @param {n} rotate left n.
	 *
	 * @return the number after rotated left.
	 */
	#define MD5_ROTATELEFT(num, n) (((num) << (n)) | ((num) >> (32-(n))))

	/**
	 * @Transformations for rounds 1, 2, 3, and 4.
	 */
	#define MD5_FF(a, b, c, d, x, s, ac) { \
	  (a) += MD5_F ((b), (c), (d)) + (x) + ac; \
	  (a) = MD5_ROTATELEFT ((a), (s)); \
	  (a) += (b); \
	}
	#define MD5_GG(a, b, c, d, x, s, ac) { \
	  (a) += MD5_G ((b), (c), (d)) + (x) + ac; \
	  (a) = MD5_ROTATELEFT ((a), (s)); \
	  (a) += (b); \
	}
	#define MD5_HH(a, b, c, d, x, s, ac) { \
	  (a) += MD5_H ((b), (c), (d)) + (x) + ac; \
	  (a) = MD5_ROTATELEFT ((a), (s)); \
	  (a) += (b); \
	}
	#define MD5_II(a, b, c, d, x, s, ac) { \
	  (a) += MD5_I ((b), (c), (d)) + (x) + ac; \
	  (a) = MD5_ROTATELEFT ((a), (s)); \
	  (a) += (b); \
	}

	class md5
	{
	public:
		/**
		 * @construct Construct a MD5 object with a std::string.
		 */
		md5(const std::string & message)
		{
			/* Initialization the object according to message. */
			this->init((const unsigned char*)message.data(), message.length());
		}

		/**
		 * @construct Construct a MD5 object with a char pointer.
		 */
		md5(const char * message)
		{
			/* Initialization the object according to message. */
			this->init((const unsigned char*)message, std::strlen(message));
		}

		/**
		 * @construct Construct a MD5 object with a unsigned char pointer.
		 */
		md5(const unsigned char * message, size_t size)
		{
			/* Initialization the object according to message. */
			this->init(message, size);
		}

		/**
		 * @destruct
		 */
		virtual ~md5()
		{
		}

		/* Generate md5 digest. */
		const unsigned char* get_digest()
		{
			if (!finished)
			{
				finished = true;

				unsigned char bits[8];
				unsigned int oldState[4];
				unsigned int oldCount[2];
				unsigned int index, padLen;

				/* Save current state and count. */
				std::memcpy(oldState, state, 16);
				std::memcpy(oldCount, count, 8);

				/* Save number of bits */
				encode(count, bits, 8);

				/* Pad out to 56 mod 64. */
				index = (unsigned int)((count[0] >> 3) & 0x3f);
				padLen = (index < 56) ? (56 - index) : (120 - index);
				this->init(PADDING, padLen);

				/* Append length (before padding) */
				this->init(bits, 8);

				/* Store state in digest */
				encode(state, digest, 16);

				/* Restore current state and count. */
				std::memcpy(state, oldState, 16);
				std::memcpy(count, oldCount, 8);
			}
			return digest;
		}

		/* Convert digest to std::string value */
		std::string str()
		{
			const unsigned char* digest_ = get_digest();
			std::string str;
			str.reserve(16 << 1);
			for (size_t i = 0; i < 16; ++i)
			{
				int t = digest_[i];
				int a = t / 16;
				int b = t % 16;
				str.append(1, HEX_NUMBERS[a]);
				str.append(1, HEX_NUMBERS[b]);
			}
			return str;
		}

	private:
		/* Initialization the md5 object, processing another message block,
		 * and updating the context.*/
		void init(const unsigned char* input, size_t len)
		{
			unsigned int i, index, partLen;

			finished = false;

			/* Compute number of bytes mod 64 */
			index = (unsigned int)((count[0] >> 3) & 0x3f);

			/* update number of bits */
			if ((count[0] += ((unsigned int)len << 3)) < ((unsigned int)len << 3)) {
				++count[1];
			}
			count[1] += ((unsigned int)len >> 29);

			partLen = 64 - index;

			/* transform as many times as possible. */
			if (len >= partLen) {

				std::memcpy(&buffer[index], input, partLen);
				transform(buffer);

				for (i = partLen; i + 63 < len; i += 64) {
					transform(&input[i]);
				}
				index = 0;

			}
			else {
				i = 0;
			}

			/* Buffer remaining input */
			std::memcpy(&buffer[index], &input[i], len - i);
		}

		/* MD5 basic transformation. Transforms state based on block. */
		void transform(const unsigned char block[64])
		{

			unsigned int a = state[0], b = state[1], c = state[2], d = state[3], x[16];

			decode(block, x, 64);

			/* Round 1 */
			MD5_FF(a, b, c, d, x[0], MD5_S11, 0xd76aa478);
			MD5_FF(d, a, b, c, x[1], MD5_S12, 0xe8c7b756);
			MD5_FF(c, d, a, b, x[2], MD5_S13, 0x242070db);
			MD5_FF(b, c, d, a, x[3], MD5_S14, 0xc1bdceee);
			MD5_FF(a, b, c, d, x[4], MD5_S11, 0xf57c0faf);
			MD5_FF(d, a, b, c, x[5], MD5_S12, 0x4787c62a);
			MD5_FF(c, d, a, b, x[6], MD5_S13, 0xa8304613);
			MD5_FF(b, c, d, a, x[7], MD5_S14, 0xfd469501);
			MD5_FF(a, b, c, d, x[8], MD5_S11, 0x698098d8);
			MD5_FF(d, a, b, c, x[9], MD5_S12, 0x8b44f7af);
			MD5_FF(c, d, a, b, x[10], MD5_S13, 0xffff5bb1);
			MD5_FF(b, c, d, a, x[11], MD5_S14, 0x895cd7be);
			MD5_FF(a, b, c, d, x[12], MD5_S11, 0x6b901122);
			MD5_FF(d, a, b, c, x[13], MD5_S12, 0xfd987193);
			MD5_FF(c, d, a, b, x[14], MD5_S13, 0xa679438e);
			MD5_FF(b, c, d, a, x[15], MD5_S14, 0x49b40821);

			/* Round 2 */
			MD5_GG(a, b, c, d, x[1], MD5_S21, 0xf61e2562);
			MD5_GG(d, a, b, c, x[6], MD5_S22, 0xc040b340);
			MD5_GG(c, d, a, b, x[11], MD5_S23, 0x265e5a51);
			MD5_GG(b, c, d, a, x[0], MD5_S24, 0xe9b6c7aa);
			MD5_GG(a, b, c, d, x[5], MD5_S21, 0xd62f105d);
			MD5_GG(d, a, b, c, x[10], MD5_S22, 0x2441453);
			MD5_GG(c, d, a, b, x[15], MD5_S23, 0xd8a1e681);
			MD5_GG(b, c, d, a, x[4], MD5_S24, 0xe7d3fbc8);
			MD5_GG(a, b, c, d, x[9], MD5_S21, 0x21e1cde6);
			MD5_GG(d, a, b, c, x[14], MD5_S22, 0xc33707d6);
			MD5_GG(c, d, a, b, x[3], MD5_S23, 0xf4d50d87);
			MD5_GG(b, c, d, a, x[8], MD5_S24, 0x455a14ed);
			MD5_GG(a, b, c, d, x[13], MD5_S21, 0xa9e3e905);
			MD5_GG(d, a, b, c, x[2], MD5_S22, 0xfcefa3f8);
			MD5_GG(c, d, a, b, x[7], MD5_S23, 0x676f02d9);
			MD5_GG(b, c, d, a, x[12], MD5_S24, 0x8d2a4c8a);

			/* Round 3 */
			MD5_HH(a, b, c, d, x[5], MD5_S31, 0xfffa3942);
			MD5_HH(d, a, b, c, x[8], MD5_S32, 0x8771f681);
			MD5_HH(c, d, a, b, x[11], MD5_S33, 0x6d9d6122);
			MD5_HH(b, c, d, a, x[14], MD5_S34, 0xfde5380c);
			MD5_HH(a, b, c, d, x[1], MD5_S31, 0xa4beea44);
			MD5_HH(d, a, b, c, x[4], MD5_S32, 0x4bdecfa9);
			MD5_HH(c, d, a, b, x[7], MD5_S33, 0xf6bb4b60);
			MD5_HH(b, c, d, a, x[10], MD5_S34, 0xbebfbc70);
			MD5_HH(a, b, c, d, x[13], MD5_S31, 0x289b7ec6);
			MD5_HH(d, a, b, c, x[0], MD5_S32, 0xeaa127fa);
			MD5_HH(c, d, a, b, x[3], MD5_S33, 0xd4ef3085);
			MD5_HH(b, c, d, a, x[6], MD5_S34, 0x4881d05);
			MD5_HH(a, b, c, d, x[9], MD5_S31, 0xd9d4d039);
			MD5_HH(d, a, b, c, x[12], MD5_S32, 0xe6db99e5);
			MD5_HH(c, d, a, b, x[15], MD5_S33, 0x1fa27cf8);
			MD5_HH(b, c, d, a, x[2], MD5_S34, 0xc4ac5665);

			/* Round 4 */
			MD5_II(a, b, c, d, x[0], MD5_S41, 0xf4292244);
			MD5_II(d, a, b, c, x[7], MD5_S42, 0x432aff97);
			MD5_II(c, d, a, b, x[14], MD5_S43, 0xab9423a7);
			MD5_II(b, c, d, a, x[5], MD5_S44, 0xfc93a039);
			MD5_II(a, b, c, d, x[12], MD5_S41, 0x655b59c3);
			MD5_II(d, a, b, c, x[3], MD5_S42, 0x8f0ccc92);
			MD5_II(c, d, a, b, x[10], MD5_S43, 0xffeff47d);
			MD5_II(b, c, d, a, x[1], MD5_S44, 0x85845dd1);
			MD5_II(a, b, c, d, x[8], MD5_S41, 0x6fa87e4f);
			MD5_II(d, a, b, c, x[15], MD5_S42, 0xfe2ce6e0);
			MD5_II(c, d, a, b, x[6], MD5_S43, 0xa3014314);
			MD5_II(b, c, d, a, x[13], MD5_S44, 0x4e0811a1);
			MD5_II(a, b, c, d, x[4], MD5_S41, 0xf7537e82);
			MD5_II(d, a, b, c, x[11], MD5_S42, 0xbd3af235);
			MD5_II(c, d, a, b, x[2], MD5_S43, 0x2ad7d2bb);
			MD5_II(b, c, d, a, x[9], MD5_S44, 0xeb86d391);

			state[0] += a;
			state[1] += b;
			state[2] += c;
			state[3] += d;
		}

		/* Encodes input (usigned long) into output (unsigned char). */
		void encode(const unsigned int* input, unsigned char* output, size_t length)
		{

			for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
				output[j] = (unsigned char)(input[i] & 0xff);
				output[j + 1] = (unsigned char)((input[i] >> 8) & 0xff);
				output[j + 2] = (unsigned char)((input[i] >> 16) & 0xff);
				output[j + 3] = (unsigned char)((input[i] >> 24) & 0xff);
			}
		}

		/* Decodes input (unsigned char) into output (usigned long). */
		void decode(const unsigned char* input, unsigned int* output, size_t length)
		{
			for (size_t i = 0, j = 0; j < length; ++i, j += 4) {
				output[i] = ((unsigned int)input[j]) | (((unsigned int)input[j + 1]) << 8) |
					(((unsigned int)input[j + 2]) << 16) | (((unsigned int)input[j + 3]) << 24);
			}
		}

	private:
		/* Flag for mark whether calculate finished. */
		bool finished = false;

		/* state (ABCD). */
		unsigned int state[4] = { 0x67452301 ,0xefcdab89 ,0x98badcfe ,0x10325476 };

		/* number of bits, low-order word first. */
		unsigned int count[2] = { 0 };

		/* input buffer. */
		unsigned char buffer[64] = { 0 };

		/* message digest. */
		unsigned char digest[16] = { 0 };

		/* padding for calculate. */
		const unsigned char PADDING[64] = { 0x80 };

		/* Hex numbers. */
		const char HEX_NUMBERS[16] = {
			'0', '1', '2', '3',
			'4', '5', '6', '7',
			'8', '9', 'a', 'b',
			'c', 'd', 'e', 'f'
		};
	};

#endif // !__ASIO2_MD5_IMPL_HPP__
