/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * https://www.ietf.org/rfc/rfc1867.txt
 * https://tools.ietf.org/html/rfc1867#section-7
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_MULTIPART_HPP__
#define __ASIO2_HTTP_MULTIPART_HPP__

#include <asio2/base/detail/push_options.hpp>

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <list>
#include <map>
#include <string>
#include <string_view>
#include <algorithm>
#include <type_traits>

#include <asio2/external/asio.hpp>
#include <asio2/external/beast.hpp>
#include <asio2/external/throw_exception.hpp>

#include <asio2/util/string.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
#define LF '\n'
#define CR '\r'
#define CRLF "\r\n"

template<class String>
class basic_multipart_field
{
public:
	/// Constructor
	basic_multipart_field() = default;
	/// Constructor
	basic_multipart_field(basic_multipart_field&&) noexcept = default;
	/// Constructor
	basic_multipart_field(basic_multipart_field const&) = default;
	/// Assignment
	basic_multipart_field& operator=(basic_multipart_field&&) noexcept = default;
	/// Assignment
	basic_multipart_field& operator=(basic_multipart_field const&) = default;

	inline const String&     content_disposition      ()      { return content_disposition_      ; }
	inline const String&     name                     ()      { return name_                     ; }
	inline const String&     value                    ()      { return value_                    ; }
	inline const String&     content_type             ()      { return content_type_             ; }
	inline const String&     filename                 ()      { return filename_                 ; }
	inline const String&     content_transfer_encoding()      { return content_transfer_encoding_; }

	inline const String& get_content_disposition      ()      { return content_disposition_      ; }
	inline const String& get_name                     ()      { return name_                     ; }
	inline const String& get_value                    ()      { return value_                    ; }
	inline const String& get_content_type             ()      { return content_type_             ; }
	inline const String& get_filename                 ()      { return filename_                 ; }
	inline const String& get_content_transfer_encoding()      { return content_transfer_encoding_; }

	inline const String&     content_disposition      () const { return content_disposition_      ; }
	inline const String&     name                     () const { return name_                     ; }
	inline const String&     value                    () const { return value_                    ; }
	inline const String&     content_type             () const { return content_type_             ; }
	inline const String&     filename                 () const { return filename_                 ; }
	inline const String&     content_transfer_encoding() const { return content_transfer_encoding_; }

	inline const String& get_content_disposition      () const { return content_disposition_      ; }
	inline const String& get_name                     () const { return name_                     ; }
	inline const String& get_value                    () const { return value_                    ; }
	inline const String& get_content_type             () const { return content_type_             ; }
	inline const String& get_filename                 () const { return filename_                 ; }
	inline const String& get_content_transfer_encoding() const { return content_transfer_encoding_; }

	template<class Str> inline basic_multipart_field&     content_disposition      (Str&& v) { content_disposition_       = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field&     name                     (Str&& v) { name_                      = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field&     value                    (Str&& v) { value_                     = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field&     content_type             (Str&& v) { content_type_              = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field&     filename                 (Str&& v) { filename_                  = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field&     content_transfer_encoding(Str&& v) { content_transfer_encoding_ = std::forward<Str>(v); return (*this); }

	template<class Str> inline basic_multipart_field& set_content_disposition      (Str&& v) { content_disposition_       = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field& set_name                     (Str&& v) { name_                      = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field& set_value                    (Str&& v) { value_                     = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field& set_content_type             (Str&& v) { content_type_              = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field& set_filename                 (Str&& v) { filename_                  = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_field& set_content_transfer_encoding(Str&& v) { content_transfer_encoding_ = std::forward<Str>(v); return (*this); }

	inline bool is_empty() const
	{
		return (
			content_disposition_      .empty() && 
			name_                     .empty() &&
			value_                    .empty() &&
			content_type_             .empty() &&
			filename_                 .empty() &&
			content_transfer_encoding_.empty()    );
	}

	inline bool empty() const
	{
		return this->is_empty();
	}

protected:
	String content_disposition_;
	String name_;
	String value_;
	String content_type_;
	String filename_;
	String content_transfer_encoding_;
};

using multipart_field = basic_multipart_field<std::string>;


template<class String>
class basic_multipart_fields
{
public:
	/// Constructor
	basic_multipart_fields() = default;
	/// Constructor
	basic_multipart_fields(basic_multipart_fields&&) = default;
	/// Constructor
	basic_multipart_fields(basic_multipart_fields const&) = default;
	/// Assignment
	basic_multipart_fields& operator=(basic_multipart_fields&&) = default;
	/// Assignment
	basic_multipart_fields& operator=(basic_multipart_fields const&) = default;

	using const_iterator = typename std::list<basic_multipart_field<String>>::const_iterator;
	using iterator = typename std::list<basic_multipart_field<String>>::iterator;

	inline const String&     boundary()       { return boundary_; }
	inline const String&     boundary() const { return boundary_; }
	inline const String& get_boundary()       { return boundary_; }
	inline const String& get_boundary() const { return boundary_; }

	template<class Str> inline basic_multipart_fields&     boundary(Str&& v) { boundary_ = std::forward<Str>(v); return (*this); }
	template<class Str> inline basic_multipart_fields& set_boundary(Str&& v) { boundary_ = std::forward<Str>(v); return (*this); }

    /** Returns the value for a field, or throws an exception.

        If more than one field with the specified name exists, the
        first field defined by insertion order is returned.

        @throws std::out_of_range if the field is not found.
     */
	inline const basic_multipart_field<String>& at(const String& name)
	{
		auto it = find(name);
		if (it == cend())
			ASIO2_THROW_EXCEPTION(std::out_of_range{ "field not found" });
		return (*it);
	}

    /*
	 * Returns the value by name, or `""` if it does not exist.
     */
	inline const basic_multipart_field<String>& operator[](const String& name)
	{
		auto it = find(name);
		if (it == cend())
			return dummy_;
		return (*it);
	}

	/// Return a const iterator to the beginning of the field sequence.
	inline iterator begin() noexcept
	{
		return list_.begin();
	}

	/// Return a const iterator to the end of the field sequence.
	inline iterator end() noexcept
	{
		return list_.end();
	}

	/// Return a const iterator to the beginning of the field sequence.
	inline const_iterator begin() const noexcept
	{
		return list_.begin();
	}

	/// Return a const iterator to the end of the field sequence.
	inline const_iterator end() const noexcept
	{
		return list_.end();
	}

	/// Return a const iterator to the beginning of the field sequence.
	inline const_iterator cbegin() const noexcept
	{
		return list_.cbegin();
	}

	/// Return a const iterator to the end of the field sequence.
	inline const_iterator cend() const noexcept
	{
		return list_.cend();
	}

    /*
	 * Remove all fields from the container
     */
	inline void clear() noexcept
	{
		set_.clear();
		list_.clear();
	}

    /*
	 * Insert a field.
     */
	template<class String1, class String2>
	inline iterator insert(String1&& name, String2&& value)
	{
		basic_multipart_field<String> field;
		field.name (std::forward<String1>(name ));
		field.value(std::forward<String2>(value));
		auto iter = set_.emplace(field.name(), std::addressof(field));
		auto itel = list_.insert(std::next(list_.begin(), std::distance(set_.begin(), iter)), std::move(field));
		iter->second = itel.operator->();
		return itel;
	}

    /*
	 * Insert a field.
     */
	inline iterator insert(basic_multipart_field<String> field)
	{
		auto iter = set_.emplace(field.name(), std::addressof(field));
		auto itel = list_.insert(std::next(list_.begin(), std::distance(set_.begin(), iter)), std::move(field));
		iter->second = itel.operator->();
		return itel;
	}

    /*
	 * Set a field value, removing any other instances of that field.
     */
	template<class String1, class String2>
	inline iterator set(String1&& name, String2&& value)
	{
		basic_multipart_field<String> field;
		field.name (std::forward<String1>(name ));
		field.value(std::forward<String2>(value));
		erase(field.name());
		auto iter = set_.emplace(field.name(), std::addressof(field));
		auto itel = list_.insert(std::next(list_.begin(), std::distance(set_.begin(), iter)), std::move(field));
		iter->second = itel.operator->();
		return itel;
	}

    /*
	 * Remove a field.
     */
	inline const_iterator erase(const_iterator pos)
	{
		auto next = pos;
		auto iter = set_.erase(std::next(set_.begin(), std::distance(list_.cbegin(), pos)));
		auto itel = list_.erase(pos);
		return (++next);
	}

    /*
	 * Remove all fields with the specified name.
     */
	inline std::size_t erase(const String& name)
	{
		auto result = set_.equal_range(name);
		if (result.first == result.second)
			return std::size_t(0);
		list_.erase(std::next(list_.begin(), std::distance(set_.begin(), result.first)),
			std::next(list_.begin(), std::distance(set_.begin(), result.second)));
		set_.erase(result.first, result.second);
		return std::distance(result.first, result.second);
	}

    //--------------------------------------------------------------------------
    //
    // Lookup
    //
    //--------------------------------------------------------------------------

    /*
	 * Return the number of fields with the specified name.
     */
	inline std::size_t count(const String& name) const
	{
		return set_.count(name);
	}

    /*
	 * Find the multipart field iterator by name.
     */
	inline iterator find(const String& name)
	{
		auto it = set_.find(name);
		if (it == set_.end())
			return list_.end();
		return std::next(list_.begin(), std::distance(set_.begin(), it));
	}

    /*
	 * Find the multipart field iterator by name.
     */
	inline const_iterator find(const String& name) const
	{
		auto it = set_.find(name);
		if (it == set_.end())
			return list_.cend();
		return std::next(list_.cbegin(), std::distance(set_.begin(), it));
	}

    /*
	 * Returns a range of iterators to the fields with the specified name.
     */
	inline std::pair<const_iterator, const_iterator> equal_range(const String& name) const
	{
		auto result = set_.equal_range(name);
		if (result.first == result.second)
			return { list_.cend(), list_.cend() };
		return {
			std::next(list_.cbegin(),std::distance(set_.begin(),result.first)),
			std::next(list_.cbegin(),std::distance(set_.begin(),result.second)) };
	}

protected:
	String                                                boundary_;
	std::list<basic_multipart_field<String>>              list_;
	std::multimap<String, basic_multipart_field<String>*> set_;

	inline static basic_multipart_field<String>           dummy_{};
};

using multipart_fields = basic_multipart_fields<std::string>;


/*
 * Convert a multipart fields to a string.
 */
template<class String>
inline std::string to_string(const basic_multipart_fields<String>& fields)
{
	std::string body;
	std::string::size_type size = 0;
	for (auto it = fields.begin(); it != fields.end(); ++it)
	{
		size += fields.boundary().size() + 2 + 2;
		size += it->content_disposition      ().size() + 22;
		size += it->name                     ().size() + 10;
		size += it->value                    ().size() + 4;
		size += it->content_type             ().size() + (it->content_type             ().empty() ? 0 : 16);
		size += it->filename                 ().size() + (it->filename                 ().empty() ? 0 : 12);
		size += it->content_transfer_encoding().size() + (it->content_transfer_encoding().empty() ? 0 : 29);
	}
	body.reserve(size + fields.boundary().size() + 2 + 2 + 2);
	for (auto it = fields.begin(); it != fields.end(); ++it)
	{
		body += "--";
		body += fields.boundary();
		body += CRLF;
		body += "Content-Disposition: ";
		body += it->content_disposition();
		if (!it->name().empty())
		{
			body += "; ";
			body += "name=\"";
			body += it->name();
			body += "\"";
		}
		if (!it->filename().empty())
		{
			body += "; ";
			body += "filename=\"";
			body += it->filename();
			body += "\"";
		}
		body += CRLF;
		if (!it->content_type().empty())
		{
			body += "Content-Type: ";
			body += it->content_type();
			body += CRLF;
		}
		if (!it->content_transfer_encoding().empty())
		{
			body += "Content-Transfer-Encoding: ";
			body += it->content_transfer_encoding();
			body += CRLF;
		}
		body += CRLF;
		if (!it->value().empty())
		{
			body += it->value();
		}
		body += CRLF;
	}
	body += "--";
	body += fields.boundary();
	body += "--";
	body += CRLF;
	return body;
}

/// Write the text for a multipart fields to an output stream.
template<class String>
inline std::ostream& operator<<(std::ostream& os, const basic_multipart_fields<String>& fields)
{
	return os << to_string(fields);
}

namespace multipart_parser
{
	template<class String>
	inline bool parse_field(basic_multipart_field<String>& field, std::string_view content)
	{
		// 8 == "\r\n" "\r\n\r\n" "\r\n"
		if (content.size() < 8)
			return false;

		// first 2 bytes must be "\r\n"
		if (content.substr(0, 2) != CRLF)
			return false;

		// last 2 bytes must be "\r\n"
		if (content.substr(content.size() - 2) != CRLF)
			return false;

		// remove the first "\r\n" and the last "\r\n"
		content = content.substr(2, content.size() - 4);

		// find the split of header and value
		auto split = content.find("\r\n\r\n");
		if (split == std::string_view::npos)
			return false;

		std::string_view header = content.substr(0, split);
		std::string_view value  = content.substr(split + 4);

		std::string_view::size_type pos_row_1 = static_cast<std::string_view::size_type>( 0);
		std::string_view::size_type pos_row_2 = static_cast<std::string_view::size_type>(-2);

		for(;;)
		{
			pos_row_1 = pos_row_2 + 2;
			pos_row_2 = header.find("\r\n", pos_row_1);

			std::string_view header_row = header.substr(pos_row_1,
				pos_row_2 == std::string_view::npos ? pos_row_2 : pos_row_2 - pos_row_1);

			// find the header type name.
			auto pos1 = header_row.find(':');
			if (pos1 == std::string_view::npos)
				return false;

			// get the header type value.
			std::string_view type = header_row.substr(0, pos1);
			asio2::trim_both(type);

			++pos1;

			if /**/(beast::iequals(type, "Content-Disposition"))
			{
				auto pos2 = header_row.find(';', pos1);

				std::string_view disposition = header_row.substr(pos1, pos2 == std::string_view::npos ? pos2 : pos2 - pos1);
				asio2::trim_both(disposition);
				field.content_disposition(disposition);

				if (pos2 != std::string_view::npos)
				{
					std::string_view kvs = header_row.substr(pos2 + 1);

					for (pos2 = 0;;)
					{
						auto pos3 = kvs.find(';', pos2);

						std::string_view kv = kvs.substr(pos2, pos3 == std::string_view::npos ? pos3 : pos3 - pos2);

						auto pos4 = kv.find('=');
						if (pos4 == std::string_view::npos)
							return false;

						std::string_view k = kv.substr(0, pos4);
						std::string_view v = kv.substr(pos4 + 1);

						asio2::trim_both(k);
						asio2::trim_both(v);

						if (!v.empty() && v.front() == '\"') v.remove_prefix(1);
						if (!v.empty() && v.back()  == '\"') v.remove_suffix(1);

						if /**/ (beast::iequals(k, "name"))
							field.name(v);
						else if (beast::iequals(k, "filename"))
							field.filename(v);

						if (pos3 == std::string_view::npos)
							break;

						pos2 = pos3 + 1;
					}
				}
			}
			else if (beast::iequals(type, "Content-Type"))
			{
				field.content_type(header_row.substr(pos1 + 1));
			}
			else if (beast::iequals(type, "Content-Transfer-Encoding"))
			{
				field.content_transfer_encoding(header_row.substr(pos1 + 1));
			}
			else
			{
				ASIO2_ASSERT(false);
			}

			if (pos_row_2 == std::string_view::npos)
				break;
		}

		field.value(value);

		return true;
	}
}

template<class String = std::string>
basic_multipart_fields<String> multipart_parser_execute(std::string_view body, std::string_view boundary)
{
	using namespace multipart_parser;

	basic_multipart_fields<String> fields{};

	fields.boundary(boundary);

	std::string full_boundary{ "--" }; full_boundary += boundary;

	std::string_view bound = full_boundary;

	for (std::size_t i = 0; i < body.size();)
	{
		basic_multipart_field<String> field{};

		// find first boundary
		if (body.substr(i, bound.size()) != bound)
			break;

		i += bound.size();

		// check whether is the end.
		std::string_view tail = body.substr(i + 0, 2);
		if (tail == "--")
		{
			i += 2;

			if (i < body.size())
			{
				if (body[i] != CR)
					break;
				++i;
			}
			if (i < body.size())
			{
				if (body[i] != LF)
					break;
				++i;
			}

			break;
		}

		// find next boundary
		auto next = body.find(bound, i);
		if (next == std::string_view::npos)
			break;

		// field.
		std::string_view field_content = body.substr(i, next - i);
		if (!parse_field(field, field_content))
			break;

		fields.insert(std::move(field));

		i = next;
	}

	return fields;
}

template<bool isRequest, class Body, class Fields, class String = std::string>
basic_multipart_fields<String> multipart_parser_execute(const http::message<isRequest, Body, Fields>& msg)
{
	std::string_view type = msg[http::field::content_type];

	std::size_t pos1 = asio2::ifind(type, "multipart/form-data");
	if (pos1 == std::string_view::npos)
		return {};
	pos1 += 19; // std::strlen("multipart/form-data");

	pos1 = asio2::ifind(type, "boundary", pos1);
	if (pos1 == std::string_view::npos)
		return {};
	pos1 += 8; // std::strlen("boundary");

	pos1 = type.find('=', pos1);
	if (pos1 == std::string_view::npos)
		return {};
	pos1 += 1;

	std::size_t pos2 = type.find_first_of("\r;", pos1);

	std::string_view boundary = type.substr(pos1, pos2 == std::string_view::npos ? pos2 : pos2 - pos1);

	return multipart_parser_execute<String>(msg.body(), boundary);
}

#undef CRLF
#undef LF
#undef CR

}

#include <asio2/base/detail/pop_options.hpp>

#endif
