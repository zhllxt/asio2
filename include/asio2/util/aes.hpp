/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from https://github.com/kokke/tiny-AES-c
 */

#ifndef __ASIO2_AES_IMPL_HPP__
#define __ASIO2_AES_IMPL_HPP__

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <sstream>

namespace asio2
{
	class aes
	{
	protected:
		// state - array holding the intermediate results during decryption.
		typedef uint8_t state_t[4][4];

		// Block length in bytes - AES is 128b block only
		static constexpr int AES_BLOCKLEN = 16;

	public:
		enum class mode_t
		{
			cbc,
			ecb,
			ctr,
			//ocf, // not supported
			//cfb, // not supported
		};

	public:
		/*
		 * if key.size() <= 16, key will be resized to 16 and padded with '\0', the data block is 128 bit.
		 * if key.size() > 16 && <= 24, key will be resized to 24 and padded with '\0', the data block is 192 bit.
		 * if key.size() > 24, key will be resized to 32 and padded with '\0', the data block is 256 bit.
		 */
		explicit aes(std::string key, mode_t mode = mode_t::ecb) : key_(std::move(key)), mode_(mode)
		{
			init();
		}
		~aes()
		{
		}

		aes(const aes & other) : key_(other.key_)
		{
			init();
		}

		aes & operator=(const aes & other)
		{
			key_ = other.key_;
			init();
			return (*this);
		}

		aes(aes && other) : key_(std::move(other.key_))
		{
			init();
		}

		aes & operator=(aes && other)
		{
			key_ = std::move(other.key_);
			init();
			return (*this);
		}

		mode_t     mode() { return mode_; }
		mode_t set_mode() { return mode_; }

		aes &     mode(mode_t mode) { mode_ = mode; return (*this); }
		aes & get_mode(mode_t mode) { mode_ = mode; return (*this); }

		aes & iv(uint8_t iv[AES_BLOCKLEN]) { memcpy(&Iv_[0], iv, AES_BLOCKLEN); return (*this); }

		/*
		 * note : if msg contains '\0',there may be a wrong result when decrypt
		 */
		std::string encrypt(std::string msg)
		{
			if (msg.empty())
				return std::string{};

			if ((msg.size() % AES_BLOCKLEN) != 0)
			{
				msg.resize(msg.size() + AES_BLOCKLEN - (msg.size() % AES_BLOCKLEN));
			}

			switch (mode_)
			{
			case mode_t::cbc: return encrypt_with_cbc(std::move(msg));
			case mode_t::ecb: return encrypt_with_ecb(std::move(msg));
			case mode_t::ctr: return encrypt_with_ctr(std::move(msg));
			}
			return std::string{};
		}
		std::string decrypt(std::string msg)
		{
			if (msg.empty() || (msg.size() % AES_BLOCKLEN) != 0)
				return std::string{};

			std::string s{};
			switch (mode_)
			{
			case mode_t::cbc: s = decrypt_with_cbc(std::move(msg)); break;
			case mode_t::ecb: s = decrypt_with_ecb(std::move(msg)); break;
			case mode_t::ctr: s = decrypt_with_ctr(std::move(msg)); break;
			}

			while (!s.empty() && s.back() == '\0')
				s.erase(s.size() - 1);

			return s;
		}

	protected:
		std::string encrypt_with_cbc(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			AES_CBC_encrypt_buffer((uint8_t*)msg.data(), uint32_t(msg.size()));

			return msg;
		}

		std::string decrypt_with_cbc(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			AES_CBC_decrypt_buffer((uint8_t*)msg.data(), uint32_t(msg.size()));

			return msg;
		}

		std::string encrypt_with_ecb(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			uint8_t * buf = (uint8_t*)msg.data();
			for (std::size_t i = 0; i < msg.size(); i += AES_BLOCKLEN)
			{
				AES_ECB_encrypt(buf);

				buf += AES_BLOCKLEN;
			}

			return msg;
		}

		std::string decrypt_with_ecb(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			uint8_t * buf = (uint8_t*)msg.data();
			for (std::size_t i = 0; i < msg.size(); i += AES_BLOCKLEN)
			{
				AES_ECB_decrypt(buf);

				buf += AES_BLOCKLEN;
			}

			return msg;
		}

		std::string encrypt_with_ctr(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			AES_CTR_xcrypt_buffer((uint8_t*)msg.data(), uint32_t(msg.size()));

			return msg;
		}

		std::string decrypt_with_ctr(std::string msg)
		{
			AES_init_ctx((const uint8_t*)key_.data());

			AES_CTR_xcrypt_buffer((uint8_t*)msg.data(), uint32_t(msg.size()));

			return msg;
		}

	protected:
		void init()
		{
			if (key_.size() <= std::size_t(16)) // 128/8
			{
				key_.resize(16);
				Nk = 4;       // The number of 32 bit words in a key.
				Nr = 10;       // The number of rounds in AES Cipher.
				RoundKey_.resize(176);
			}
			else if (key_.size() <= std::size_t(24)) // 192/8
			{
				key_.resize(24);
				Nk = 6;
				Nr = 12;
				RoundKey_.resize(208);
			}
			else// 256/8
			{
				key_.resize(32);
				Nk = 8;
				Nr = 14;
				RoundKey_.resize(240);
			}
		}

		void AES_init_ctx(const uint8_t* key)
		{
			KeyExpansion(&RoundKey_[0], key);
		}
		void AES_init_ctx_iv(const uint8_t* key, const uint8_t* iv)
		{
			KeyExpansion(&RoundKey_[0], key);
			memcpy(&Iv_[0], iv, AES_BLOCKLEN);
		}
		void AES_ctx_set_iv(const uint8_t* iv)
		{
			memcpy(&Iv_[0], iv, AES_BLOCKLEN);
		}

		void AES_ECB_encrypt(uint8_t* buf)
		{
			// The next function call encrypts the PlainText with the Key using AES algorithm.
			Cipher((state_t*)buf, &RoundKey_[0]);
		}
		void AES_ECB_decrypt(uint8_t* buf)
		{
			// The next function call decrypts the PlainText with the Key using AES algorithm.
			InvCipher((state_t*)buf, &RoundKey_[0]);
		}

		void AES_CBC_encrypt_buffer(uint8_t* buf, uint32_t length)
		{
			uint32_t i;
			uint8_t *iv = &Iv_[0];
			for (i = 0; i < length; i += AES_BLOCKLEN)
			{
				XorWithIv(buf, iv);
				Cipher((state_t*)buf, &RoundKey_[0]);
				iv = buf;
				buf += AES_BLOCKLEN;
			}
			/* store Iv in ctx for next call */
			memcpy(&Iv_[0], iv, AES_BLOCKLEN);
		}
		void AES_CBC_decrypt_buffer(uint8_t* buf, uint32_t length)
		{
			uint32_t i;
			uint8_t storeNextIv[AES_BLOCKLEN];
			for (i = 0; i < length; i += AES_BLOCKLEN)
			{
				memcpy(storeNextIv, buf, AES_BLOCKLEN);
				InvCipher((state_t*)buf, &RoundKey_[0]);
				XorWithIv(buf, &Iv_[0]);
				memcpy(&Iv_[0], storeNextIv, AES_BLOCKLEN);
				buf += AES_BLOCKLEN;
			}
		}

		void AES_CTR_xcrypt_buffer(uint8_t* buf, uint32_t length)
		{
			uint8_t buffer[AES_BLOCKLEN];

			unsigned i;
			int bi;
			for (i = 0, bi = AES_BLOCKLEN; i < length; ++i, ++bi)
			{
				if (bi == AES_BLOCKLEN) /* we need to regen xor compliment in buffer */
				{
					memcpy(buffer, &Iv_[0], AES_BLOCKLEN);
					Cipher((state_t*)buffer, &RoundKey_[0]);

					/* Increment Iv and handle overflow */
					for (bi = (AES_BLOCKLEN - 1); bi >= 0; --bi)
					{
						/* inc will overflow */
						if (Iv_[bi] == 255)
						{
							Iv_[bi] = uint8_t(0);
							continue;
						}
						Iv_[bi] = uint8_t(Iv_[bi] + uint8_t(1));
						break;
					}
					bi = 0;
				}

				buf[i] = (buf[i] ^ buffer[bi]);
			}
		}

		void XorWithIv(uint8_t* buf, const uint8_t* iv)
		{
			uint8_t i;
			for (i = 0; i < AES_BLOCKLEN; ++i) // The block in AES is always 128bit no matter the key size
			{
				buf[i] ^= iv[i];
			}
		}

		inline uint8_t Multiply(uint8_t x, uint8_t y)
		{
		  return (uint8_t((((y & 1) * x) ^
			   ((y>>1 & 1) * xtime(x)) ^
			   ((y>>2 & 1) * xtime(xtime(x))) ^
			   ((y>>3 & 1) * xtime(xtime(xtime(x)))) ^
			   ((y>>4 & 1) * xtime(xtime(xtime(xtime(x)))))))); /* this last call to xtime() can be omitted */
		}

		// This function adds the round key to state.
		// The round key is added to the state by an XOR function.
		void AddRoundKey(uint8_t round, state_t* state, const uint8_t* RoundKey)
		{
		  uint8_t i,j;
		  for (i = 0; i < 4; ++i)
		  {
			for (j = 0; j < 4; ++j)
			{
			  (*state)[i][j] ^= RoundKey[(round * Nb * 4) + (i * Nb) + j];
			}
		  }
		}

		// The SubBytes Function Substitutes the values in the
		// state matrix with values in an S-box.
		void SubBytes(state_t* state)
		{
		  uint8_t i, j;
		  for (i = 0; i < 4; ++i)
		  {
			for (j = 0; j < 4; ++j)
			{
			  (*state)[j][i] = getSBoxValue((*state)[j][i]);
			}
		  }
		}

		// The ShiftRows() function shifts the rows in the state to the left.
		// Each row is shifted with different offset.
		// Offset = Row number. So the first row is not shifted.
		void ShiftRows(state_t* state)
		{
		  uint8_t temp;

		  // Rotate first row 1 columns to left  
		  temp           = (*state)[0][1];
		  (*state)[0][1] = (*state)[1][1];
		  (*state)[1][1] = (*state)[2][1];
		  (*state)[2][1] = (*state)[3][1];
		  (*state)[3][1] = temp;

		  // Rotate second row 2 columns to left  
		  temp           = (*state)[0][2];
		  (*state)[0][2] = (*state)[2][2];
		  (*state)[2][2] = temp;

		  temp           = (*state)[1][2];
		  (*state)[1][2] = (*state)[3][2];
		  (*state)[3][2] = temp;

		  // Rotate third row 3 columns to left
		  temp           = (*state)[0][3];
		  (*state)[0][3] = (*state)[3][3];
		  (*state)[3][3] = (*state)[2][3];
		  (*state)[2][3] = (*state)[1][3];
		  (*state)[1][3] = temp;
		}

		inline uint8_t xtime(uint8_t x)
		{
		  return (uint8_t(((x<<1) ^ (((x>>7) & 1) * 0x1b))));
		}

		// MixColumns function mixes the columns of the state matrix
		void MixColumns(state_t* state)
		{
		  uint8_t i;
		  uint8_t Tmp, Tm, t;
		  for (i = 0; i < 4; ++i)
		  {  
			t   = (*state)[i][0];
			Tmp = (*state)[i][0] ^ (*state)[i][1] ^ (*state)[i][2] ^ (*state)[i][3] ;
			Tm  = (*state)[i][0] ^ (*state)[i][1] ; Tm = xtime(Tm);  (*state)[i][0] ^= Tm ^ Tmp ;
			Tm  = (*state)[i][1] ^ (*state)[i][2] ; Tm = xtime(Tm);  (*state)[i][1] ^= Tm ^ Tmp ;
			Tm  = (*state)[i][2] ^ (*state)[i][3] ; Tm = xtime(Tm);  (*state)[i][2] ^= Tm ^ Tmp ;
			Tm  = (*state)[i][3] ^ t ;              Tm = xtime(Tm);  (*state)[i][3] ^= Tm ^ Tmp ;
		  }
		}

		// MixColumns function mixes the columns of the state matrix.
		// The method used to multiply may be difficult to understand for the inexperienced.
		// Please use the references to gain more information.
		void InvMixColumns(state_t* state)
		{
		  int i;
		  uint8_t a, b, c, d;
		  for (i = 0; i < 4; ++i)
		  { 
			a = (*state)[i][0];
			b = (*state)[i][1];
			c = (*state)[i][2];
			d = (*state)[i][3];

			(*state)[i][0] = Multiply(a, 0x0e) ^ Multiply(b, 0x0b) ^ Multiply(c, 0x0d) ^ Multiply(d, 0x09);
			(*state)[i][1] = Multiply(a, 0x09) ^ Multiply(b, 0x0e) ^ Multiply(c, 0x0b) ^ Multiply(d, 0x0d);
			(*state)[i][2] = Multiply(a, 0x0d) ^ Multiply(b, 0x09) ^ Multiply(c, 0x0e) ^ Multiply(d, 0x0b);
			(*state)[i][3] = Multiply(a, 0x0b) ^ Multiply(b, 0x0d) ^ Multiply(c, 0x09) ^ Multiply(d, 0x0e);
		  }
		}

		// The SubBytes Function Substitutes the values in the
		// state matrix with values in an S-box.
		void InvSubBytes(state_t* state)
		{
		  uint8_t i, j;
		  for (i = 0; i < 4; ++i)
		  {
			for (j = 0; j < 4; ++j)
			{
			  (*state)[j][i] = getSBoxInvert((*state)[j][i]);
			}
		  }
		}

		void InvShiftRows(state_t* state)
		{
		  uint8_t temp;

		  // Rotate first row 1 columns to right  
		  temp = (*state)[3][1];
		  (*state)[3][1] = (*state)[2][1];
		  (*state)[2][1] = (*state)[1][1];
		  (*state)[1][1] = (*state)[0][1];
		  (*state)[0][1] = temp;

		  // Rotate second row 2 columns to right 
		  temp = (*state)[0][2];
		  (*state)[0][2] = (*state)[2][2];
		  (*state)[2][2] = temp;

		  temp = (*state)[1][2];
		  (*state)[1][2] = (*state)[3][2];
		  (*state)[3][2] = temp;

		  // Rotate third row 3 columns to right
		  temp = (*state)[0][3];
		  (*state)[0][3] = (*state)[1][3];
		  (*state)[1][3] = (*state)[2][3];
		  (*state)[2][3] = (*state)[3][3];
		  (*state)[3][3] = temp;
		}

		inline uint8_t getSBoxValue(uint8_t num)
		{
			return sbox[num];
		}

		inline uint8_t getSBoxInvert(uint8_t num)
		{
			return rsbox[num];
		}

		// This function produces Nb(Nr+1) round keys. The round keys are used in each round to decrypt the states. 
		void KeyExpansion(uint8_t* RoundKey, const uint8_t* Key)
		{
			unsigned i, j, k;
			uint8_t tempa[4]; // Used for the column/row operations

			// The first round key is the key itself.
			for (i = 0; i < Nk; ++i)
			{
				RoundKey[(i * 4) + 0] = Key[(i * 4) + 0];
				RoundKey[(i * 4) + 1] = Key[(i * 4) + 1];
				RoundKey[(i * 4) + 2] = Key[(i * 4) + 2];
				RoundKey[(i * 4) + 3] = Key[(i * 4) + 3];
			}

			// All other round keys are found from the previous round keys.
			for (i = Nk; i < Nb * (Nr + 1); ++i)
			{
				{
					k = (i - 1) * 4;
					tempa[0] = RoundKey[k + 0];
					tempa[1] = RoundKey[k + 1];
					tempa[2] = RoundKey[k + 2];
					tempa[3] = RoundKey[k + 3];

				}

				if (i % Nk == 0)
				{
					// This function shifts the 4 bytes in a word to the left once.
					// [a0,a1,a2,a3] becomes [a1,a2,a3,a0]

					// Function RotWord()
					{
						const uint8_t u8tmp = tempa[0];
						tempa[0] = tempa[1];
						tempa[1] = tempa[2];
						tempa[2] = tempa[3];
						tempa[3] = u8tmp;
					}

					// SubWord() is a function that takes a four-byte input word and 
					// applies the S-box to each of the four bytes to produce an output word.

					// Function Subword()
					{
						tempa[0] = getSBoxValue(tempa[0]);
						tempa[1] = getSBoxValue(tempa[1]);
						tempa[2] = getSBoxValue(tempa[2]);
						tempa[3] = getSBoxValue(tempa[3]);
					}

					tempa[0] = tempa[0] ^ Rcon[i / Nk];
				}
				if (Nk == 8) // AES256
				{
					if (i % Nk == 4)
					{
						// Function Subword()
						{
							tempa[0] = getSBoxValue(tempa[0]);
							tempa[1] = getSBoxValue(tempa[1]);
							tempa[2] = getSBoxValue(tempa[2]);
							tempa[3] = getSBoxValue(tempa[3]);
						}
					}
				}
				j = i * 4; k = (i - Nk) * 4;
				RoundKey[j + 0] = RoundKey[k + 0] ^ tempa[0];
				RoundKey[j + 1] = RoundKey[k + 1] ^ tempa[1];
				RoundKey[j + 2] = RoundKey[k + 2] ^ tempa[2];
				RoundKey[j + 3] = RoundKey[k + 3] ^ tempa[3];
			}
		}

		// Cipher is the main function that encrypts the PlainText.
		void Cipher(state_t* state, const uint8_t* RoundKey)
		{
			uint8_t round = 0;

			// Add the First round key to the state before starting the rounds.
			AddRoundKey(0, state, RoundKey);

			// There will be Nr rounds.
			// The first Nr-1 rounds are identical.
			// These Nr rounds are executed in the loop below.
			// Last one without MixColumns()
			for (round = 1; ; ++round)
			{
				SubBytes(state);
				ShiftRows(state);
				if (round == Nr) {
					break;
				}
				MixColumns(state);
				AddRoundKey(round, state, RoundKey);
			}
			// Add round key to last round
			AddRoundKey(uint8_t(Nr), state, RoundKey);
		}

		void InvCipher(state_t* state, const uint8_t* RoundKey)
		{
			uint8_t round = 0;

			// Add the First round key to the state before starting the rounds.
			AddRoundKey(uint8_t(Nr), state, RoundKey);

			// There will be Nr rounds.
			// The first Nr-1 rounds are identical.
			// These Nr rounds are executed in the loop below.
			// Last one without InvMixColumn()
			for (round = uint8_t(Nr - 1); ; --round)
			{
				InvShiftRows(state);
				InvSubBytes(state);
				AddRoundKey(round, state, RoundKey);
				if (round == 0) {
					break;
				}
				InvMixColumns(state);
			}
		}

	protected:
		std::string key_;

		mode_t mode_ = mode_t::ecb;

		// The number of columns comprising a state in AES. This is a constant in AES. Value=4
		unsigned int Nb = 4;

		unsigned int Nk = 4;       // The number of 32 bit words in a key.
		unsigned int Nr = 10;      // The number of rounds in AES Cipher.

		std::vector<uint8_t> RoundKey_{};
		std::array<uint8_t, AES_BLOCKLEN> Iv_{ 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };

		// The lookup-tables are marked const so they can be placed in read-only storage instead of RAM
		// The numbers below can be computed dynamically trading ROM for RAM - 
		// This can be useful in (embedded) bootloader applications, where ROM is often limited.
		const uint8_t sbox[256] =
		{
			//0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
			0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
			0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
			0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
			0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
			0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
			0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
			0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
			0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
			0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
			0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
			0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
			0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
			0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
			0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
			0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
			0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
		};

		const uint8_t rsbox[256] =
		{
		  0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
		  0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
		  0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
		  0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
		  0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
		  0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
		  0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
		  0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
		  0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
		  0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
		  0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
		  0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
		  0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
		  0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
		  0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
		  0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
		};

		// The round constant word array, Rcon[i], contains the values given by 
		// x to the power (i-1) being powers of x (x is denoted as {02}) in the field GF(2^8)
		const uint8_t Rcon[11] =
		{
		  0x8d, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36
		};
	};
}

#endif // !__ASIO2_AES_IMPL_HPP__
