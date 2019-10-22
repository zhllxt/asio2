/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
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

namespace asio2::detail
{
	class strbuf : public std::streambuf
	{
	public:
		using string_type = std::basic_string<char_type, traits_type>;
		using size_type = typename string_type::size_type;

		strbuf() {}
		~strbuf() = default;

		inline const string_type& str() const
		{
			return this->str_;
		}
		inline void str(const string_type& s)
		{
			this->str_ = s;
		}
		inline strbuf& setbuf(std::string_view s)
		{
			this->setbuf(const_cast<char_type*>(s.data()), s.size());
			return (*this);
		}
		virtual std::streambuf* setbuf(char_type* s, std::streamsize n) override
		{
			this->setp(s, s + n);
			this->setg(s, s, s + n);
			return this;
		}

	protected:
		virtual std::streamsize xsputn(const char_type* s, std::streamsize count) override
		{
			this->str_.append(s, size_type(count));
			return count;
		}
		//virtual int_type overflow(int_type ch = traits_type::eof()) override
		//{
		//	return ch;
		//}
		//virtual int_type underflow() override
		//{
		//	return traits_type::eof();
		//}
		//virtual int_type uflow() override
		//{
		//	return traits_type::eof();
		//}
	protected:
		string_type str_;
	};

	class serializer
	{
	public:
		serializer()
			: buffer_()
			, ostream_(&buffer_)
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

		inline serializer& reset(const strbuf::string_type& s = "")
		{
			this->buffer_.str(s);
			return (*this);
		}

		inline const auto& str() const
		{
			return this->buffer_.str();
		}

		inline strbuf& buffer() { return this->buffer_; }

	protected:
		strbuf buffer_;
		std::ostream ostream_;
		cereal::binary_oarchive oarchive_;
	};

	class deserializer
	{
	public:
		deserializer()
			: buffer_()
			, istream_(&buffer_)
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
			this->buffer_.setbuf(s);
			return (*this);
		}

		inline const auto& str() const
		{
			return this->buffer_.str();
		}

		inline strbuf& buffer() { return this->buffer_; }

	protected:
		strbuf buffer_;
		std::istream istream_;
		cereal::binary_iarchive iarchive_;
	};
}

#endif // !__ASIO2_RPC_SERIALIZATION_HPP__
