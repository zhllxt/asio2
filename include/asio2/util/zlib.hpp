/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * https://github.com/r-lyeh-archived/bundle
 * 
 */

#ifndef __ASIO2_ZLIB_HPP__
#define __ASIO2_ZLIB_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <string_view>
#include <algorithm>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>

#include <asio2/base/error.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::zlib
#else
namespace boost::beast::zlib
#endif
{
	/**
	 * impl : use the beast::zlib for compress and uncompress
	 * Problem: compress data with beast::zlib and uncompress data with zlib(www.zlib.net) will fail
	 */
	class impl
	{
	public:
		impl(
			beast::zlib::Flush   compress_flush = beast::zlib::Flush::sync,
			beast::zlib::Flush uncompress_flush = beast::zlib::Flush::sync
		)
			: deflate_flush_(  compress_flush)
			, inflate_flush_(uncompress_flush)
		{
		}

		~impl() = default;

		impl(impl&&) = delete;
		impl(const impl&) = delete;
		impl& operator=(impl&&) = delete;
		impl& operator=(const impl&) = delete;

		inline std::string compress(std::string_view data)
		{
			return work(deflate_stream_, deflate_flush_, data, asio2::get_last_error());
		}

		inline std::string uncompress(std::string_view data)
		{
			return work(inflate_stream_, inflate_flush_, data, asio2::get_last_error());
		}

		inline static std::size_t compress_bound(std::size_t size)
		{
			return beast::zlib::deflate_upper_bound(size);
		}

		inline static std::size_t uncompress_bound(std::size_t size)
		{
			std::size_t bound = size;
			std::size_t deflate_bound = beast::zlib::deflate_upper_bound(bound) + 5 + 6 + 1;

			while (deflate_bound < size)
			{
				bound += (size - deflate_bound) * 2;
				deflate_bound = beast::zlib::deflate_upper_bound(bound) + 5 + 6 + 1;
			}

			return (std::max)(bound, deflate_bound);
		}

		inline beast::zlib::deflate_stream&   compressor() { return deflate_stream_; }
		inline beast::zlib::inflate_stream& uncompressor() { return inflate_stream_; }

	protected:
		template<class CompressOrUncompress>
		inline std::size_t calc_bound(CompressOrUncompress&, beast::zlib::Flush, std::size_t size)
		{
			using optype = std::remove_cv_t<std::remove_reference_t<CompressOrUncompress>>;

			if constexpr /**/ (std::is_same_v<beast::zlib::deflate_stream, optype>)
			{
				return compress_bound(size);
			}
			else if constexpr (std::is_same_v<beast::zlib::inflate_stream, optype>)
			{
				return uncompress_bound(size);
			}
			else
			{
				// http://www.purecpp.cn/detail?id=2293
				static_assert(!sizeof(CompressOrUncompress));
			}
		}

		template<class CompressOrUncompress>
		inline std::string work(CompressOrUncompress& op, beast::zlib::Flush flush, std::string_view data, asio::error_code& ec)
		{
			ec.clear();

			std::string result{};

			result.resize(calc_bound(op, flush, data.size()));

			beast::zlib::z_params zs{};

			zs.next_in   = decltype(zs.next_in)(data.data());
			zs.avail_in  = data.size();

			zs.next_out  = decltype(zs.next_out)(result.data());
			zs.avail_out = result.size();

			while (true)
			{
				op.write(zs, flush, ec);

				if (ec)
					break;

				if (zs.avail_in == std::size_t(0))
					break;

				result.resize(result.size() + calc_bound(op, flush, zs.avail_in));

				zs.next_out  = decltype(zs.next_out)(result.data() + zs.total_out);
				zs.avail_out = result.size() - zs.total_out;
			}

			if (!ec)
				result.resize(zs.total_out);
			else
				result.resize(0);

			return result;
		}

	protected:
		beast::zlib::deflate_stream deflate_stream_{};
		beast::zlib::inflate_stream inflate_stream_{};

		beast::zlib::Flush          deflate_flush_{ beast::zlib::Flush::sync };
		beast::zlib::Flush          inflate_flush_{ beast::zlib::Flush::sync };
	};
}

#endif // !__ASIO2_ZLIB_HPP__
