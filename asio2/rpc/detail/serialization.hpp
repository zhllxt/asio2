/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_SERIALIZATION_HPP__
#define __ASIO2_RPC_SERIALIZATION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <istream>
#include <ostream>
#include <streambuf>
#include <string>
#include <string_view>

#include <asio2/3rd/cereal.hpp>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>

#include <asio2/rpc/detail/rpc_portable_binary.hpp>

namespace asio2::detail
{
	class ostrbuf : public std::streambuf
	{
	public:
		using string_type = std::basic_string<char_type, traits_type>;
		using size_type = typename string_type::size_type;

		ostrbuf() {}
		virtual ~ostrbuf() {}

		inline const string_type& str() const
		{
			return this->str_;
		}

		inline void clear()
		{
			this->str_.clear();

			this->setp(this->str_.data(), this->str_.data() + this->str_.size());
		}

	protected:
		virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override
		{
			this->str_.append(s, size_type(count));

			this->setp(this->str_.data(), this->str_.data() + this->str_.size());

			return count;
		}

	protected:
		string_type str_;
	};

	class istrbuf : public std::streambuf
	{
	public:
		using string_type = std::basic_string<char_type, traits_type>;
		using size_type = typename string_type::size_type;

		istrbuf() {}
		virtual ~istrbuf() {}

		inline void setbuf(std::string_view s)
		{
			this->setbuf(const_cast<char_type*>(s.data()), std::streamsize(s.size()));
		}

		virtual std::streambuf* setbuf(char_type* s, std::streamsize n) override
		{
			this->setg(s, s, s + n);
			return this;
		}

	protected:
		virtual std::streamsize xsgetn(char_type* s, std::streamsize count) override
		{
			if (this->in_avail() < count)
				return std::streamsize(0);

			std::memcpy((void*)s, (const void*)(this->gptr()), size_type(count));

			this->setg(this->eback(), this->gptr() + count, this->egptr());

			return count;
		}
	};

	class serializer
	{
	public:
		using oarchive = cereal::RPCPortableBinaryOutputArchive;

		serializer()
			: obuffer_()
			, ostream_(&obuffer_)
			, oarchive_(ostream_)
		{}
		~serializer() = default;

		template<typename T>
		inline serializer& operator<<(const T& v)
		{
			this->oarchive_ << v;
			return (*this);
		}

		inline serializer& operator<<(const error_code& ec)
		{
			this->oarchive_ << ec.value();
			return (*this);
		}

		template<class ...Args>
		inline serializer& save(const Args&... args)
		{
			((this->oarchive_ << args), ...);
			return (*this);
		}

		inline serializer& reset()
		{
			this->obuffer_.clear();
			this->oarchive_.save_endian();
			return (*this);
		}

		inline const auto& str() const
		{
			return this->obuffer_.str();
		}

		inline ostrbuf& buffer() { return this->obuffer_; }

	protected:
		ostrbuf         obuffer_;
		std::ostream    ostream_;
		oarchive        oarchive_;
	};

	class deserializer
	{
	public:
		using iarchive = cereal::RPCPortableBinaryInputArchive;

		deserializer()
			: ibuffer_()
			, istream_(&ibuffer_)
			, iarchive_(istream_)
		{}
		~deserializer() = default;

		template<typename T>
		inline deserializer& operator>>(T& v)
		{
			this->iarchive_ >> v;
			return (*this);
		}

		inline deserializer& operator>>(error_code& ec)
		{
			decltype(ec.value()) v;
			this->iarchive_ >> v;
			ec.assign(v, ec.category());
			return (*this);
		}

		template<class ...Args>
		inline deserializer& load(Args&... args)
		{
			((this->iarchive_ >> args), ...);
			return (*this);
		}

		inline deserializer& reset(std::string_view s)
		{
			this->ibuffer_.setbuf(s);
			this->iarchive_.load_endian();
			return (*this);
		}

		inline istrbuf& buffer() { return this->ibuffer_; }

	protected:
		istrbuf         ibuffer_;
		std::istream    istream_;
		iarchive        iarchive_;
	};
}

#endif // !__ASIO2_RPC_SERIALIZATION_HPP__
