//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef __ASIO2_HTTP_FLEX_BODY_HPP__
#define __ASIO2_HTTP_FLEX_BODY_HPP__

#include <asio2/external/beast.hpp>
#include <asio2/external/assert.hpp>

#include <asio2/http/detail/http_util.hpp>

#ifdef ASIO2_HEADER_ONLY
namespace bho {
#else
namespace boost {
#endif
namespace beast {
namespace http {

/** A message body represented by a file on the filesystem.

    Messages with this type have bodies represented by a
    file on the file system. When parsing a message using
    this body type, the data is stored in the file pointed
    to by the path, which must be writable. When serializing,
    the implementation will read the file and present those
    octets as the body content. This may be used to serve
    content from a directory as part of a web service.

    @tparam File The implementation to use for accessing files.
    This type must meet the requirements of <em>File</em>.
*/
template<class TextBody, class FileBody>
struct basic_flex_body
{
    // Algorithm for storing buffers when parsing.
    class reader;

    // Algorithm for retrieving buffers when serializing.
    class writer;

    // The type of the @ref message::body member.
    class value_type;

    /** Returns the size of the body

        @param body The file body to use
    */
    static inline 
    std::uint64_t
    size(value_type const& body) noexcept;
};

/** The type of the @ref message::body member.

    Messages declared using `basic_flex_body` will have this type for
    the body member. This rich class interface allow the file to be
    opened with the file handle maintained directly in the object,
    which is attached to the message.
*/
template<class TextBody, class FileBody>
class basic_flex_body<TextBody, FileBody>::value_type
{
public:
	using text_t = typename TextBody::value_type;
	using file_t = typename FileBody::value_type;

private:
    // This body container holds a handle to the file
    // when it is open, and also caches the size when set.

    friend class reader;
    friend class writer;
    friend struct basic_flex_body;

	text_t text_;
	file_t file_;
    std::uint64_t to_text_limit_ = std::uint64_t(8 * 1024 * 1024);

public:
    /** Destructor.

        If the file is open, it is closed first.
    */
    ~value_type() = default;

    /// Constructor
    value_type() = default;

    /// Constructor
    value_type(value_type const&) = default;

    /// Assignment
    value_type& operator=(value_type const&) = default;

    /// Constructor
    value_type(value_type&& other) = default;

    /// Move assignment
    value_type& operator=(value_type&& other) = default;

	inline text_t& text() noexcept
	{
		return text_;
	}

	inline text_t& text() const noexcept
	{
		return const_cast<text_t&>(text_);
	}

    /// Return the file
	inline file_t& file() noexcept
    {
        return file_;
    }

    /// Return the file
	inline file_t& file() const noexcept
    {
        return const_cast<file_t&>(file_);
    }

    /// Returns the size of the text or file
	inline std::uint64_t size() const noexcept
    {
		return (is_file() ? file_.size() : text_.size());
    }

	inline bool is_text() const noexcept { return !is_file();      }
	inline bool is_file() const noexcept { return file_.is_open(); }

    inline void set_to_text_limit(std::uint64_t v) const noexcept
    {
        this->to_text_limit_ = v;
    }

    /// Convert the file body to text body.
    inline bool to_text()
    {
        if (this->is_text())
            return true;

		if (this->size() > this->to_text_limit_)
            return false;

        error_code ec;
        auto& f = file_.file();

        f.seek(0, ec);
        if (ec)
            return false;

        text_.resize(std::size_t(this->size()));

        auto const nread = f.read(text_.data(), text_.size(), ec);
        if (ec || nread != text_.size())
        {
            text_.clear();
            return false;
        }

        file_ = file_t{};

        return true;
    }
};

// This is called from message::payload_size
template<class TextBody, class FileBody>
std::uint64_t
basic_flex_body<TextBody, FileBody>::
size(value_type const& body) noexcept
{
    // Forward the call to the body
    return body.size();
}

/** Algorithm for retrieving buffers when serializing.

    Objects of this type are created during serialization
    to extract the buffers representing the body.
*/
template<class TextBody, class FileBody>
class basic_flex_body<TextBody, FileBody>::writer
{
public:
	using text_writer = typename TextBody::writer;
	using file_writer = typename FileBody::writer;

private:
	value_type const& body_;      // The body we are reading from
	text_writer text_writer_;
	file_writer file_writer_;

public:
    // The type of buffer sequence returned by `get`.
    //
    using const_buffers_type =
        ::asio::const_buffer;

    // Constructor.
    //
    // `h` holds the headers of the message we are
    // serializing, while `b` holds the body.
    //
    // Note that the message is passed by non-const reference.
    // This is intentional, because reading from the file
    // changes its "current position" which counts makes the
    // operation logically not-const (although it is bitwise
    // const).
    //
    // The BodyWriter concept allows the writer to choose
    // whether to take the message by const reference or
    // non-const reference. Depending on the choice, a
    // serializer constructed using that body type will
    // require the same const or non-const reference to
    // construct.
    //
    // Readers which accept const messages usually allow
    // the same body to be serialized by multiple threads
    // concurrently, while readers accepting non-const
    // messages may only be serialized by one thread at
    // a time.
    //
    template<bool isRequest, class Fields>
    writer(header<isRequest, Fields> const& h, value_type const& b);

    // Initializer
    //
    // This is called before the body is serialized and
    // gives the writer a chance to do something that might
    // need to return an error code.
    //
	inline void
    init(error_code& ec);

    // This function is called zero or more times to
    // retrieve buffers. A return value of `std::nullopt`
    // means there are no more buffers. Otherwise,
    // the contained pair will have the next buffer
    // to serialize, and a `bool` indicating whether
    // or not there may be additional buffers.
#ifdef ASIO2_HEADER_ONLY
	inline std::optional<std::pair<const_buffers_type, bool>>
#else
	inline boost::optional<std::pair<const_buffers_type, bool>>
#endif
    get(error_code& ec);
};

// Here we just stash a reference to the path for later.
// Rather than dealing with messy constructor exceptions,
// we save the things that might fail for the call to `init`.
//
template<class TextBody, class FileBody>
template<bool isRequest, class Fields>
basic_flex_body<TextBody, FileBody>::
writer::
writer(header<isRequest, Fields> const& h, value_type const& b)
	: body_(b)
	, text_writer_(const_cast<header<isRequest, Fields>&>(h), const_cast<value_type&>(b).text())
	, file_writer_(const_cast<header<isRequest, Fields>&>(h), const_cast<value_type&>(b).file())
{
}

// Initializer
template<class TextBody, class FileBody>
void
basic_flex_body<TextBody, FileBody>::
writer::
init(error_code& ec)
{
    // The error_code specification requires that we
    // either set the error to some value, or set it
    // to indicate no error.
    //
    // We don't do anything fancy so set "no error"
	if (body_.is_file())
		file_writer_.init(ec);
	else
		text_writer_.init(ec);
}

// This function is called repeatedly by the serializer to
// retrieve the buffers representing the body. Our strategy
// is to read into our buffer and return it until we have
// read through the whole file.
//
template<class TextBody, class FileBody>
auto
basic_flex_body<TextBody, FileBody>::
writer::
get(error_code& ec) ->
#ifdef ASIO2_HEADER_ONLY
    std::optional<std::pair<const_buffers_type, bool>>
#else
    boost::optional<std::pair<const_buffers_type, bool>>
#endif
{
	if (body_.is_file())
		return file_writer_.get(ec);
	else
		return text_writer_.get(ec);
}

/** Algorithm for storing buffers when parsing.

    Objects of this type are created during parsing
    to store incoming buffers representing the body.
*/
template<class TextBody, class FileBody>
class basic_flex_body<TextBody, FileBody>::reader
{
public:
	using text_reader = typename TextBody::reader;
	using file_reader = typename FileBody::reader;

private:
	value_type& body_;      // The body we are reading from
	text_reader text_reader_;
	file_reader file_reader_;

public:
    // Constructor.
    //
    // This is called after the header is parsed and
    // indicates that a non-zero sized body may be present.
    // `h` holds the received message headers.
    // `b` is an instance of `basic_flex_body`.
    //
    template<bool isRequest, class Fields>
    explicit
    reader(header<isRequest, Fields>&h, value_type& b);

    // Initializer
    //
    // This is called before the body is parsed and
    // gives the reader a chance to do something that might
    // need to return an error code. It informs us of
    // the payload size (`content_length`) which we can
    // optionally use for optimization.
    //
	inline void
#ifdef ASIO2_HEADER_ONLY
    init(std::optional<std::uint64_t> const&, error_code& ec);
#else
    init(boost::optional<std::uint64_t> const&, error_code& ec);
#endif

    // This function is called one or more times to store
    // buffer sequences corresponding to the incoming body.
    //
    template<class ConstBufferSequence>
	inline std::size_t
    put(ConstBufferSequence const& buffers,
        error_code& ec);

    // This function is called when writing is complete.
    // It is an opportunity to perform any final actions
    // which might fail, in order to return an error code.
    // Operations that might fail should not be attempted in
    // destructors, since an exception thrown from there
    // would terminate the program.
    //
	inline void
    finish(error_code& ec);
};

// We don't do much in the reader constructor since the
// file is already open.
//
template<class TextBody, class FileBody>
template<bool isRequest, class Fields>
basic_flex_body<TextBody, FileBody>::
reader::
reader(header<isRequest, Fields>& h, value_type& body)
	: body_(body), text_reader_(h, body.text()), file_reader_(h, body.file())
{
}

template<class TextBody, class FileBody>
void
basic_flex_body<TextBody, FileBody>::
reader::
init(
#ifdef ASIO2_HEADER_ONLY
    std::optional<std::uint64_t> const& content_length,
#else
    boost::optional<std::uint64_t> const& content_length,
#endif
    error_code& ec)
{
	if (body_.is_file())
		file_reader_.init(content_length, ec);
	else
		text_reader_.init(content_length, ec);
}

// This will get called one or more times with body buffers
//
template<class TextBody, class FileBody>
template<class ConstBufferSequence>
std::size_t
basic_flex_body<TextBody, FileBody>::
reader::
put(ConstBufferSequence const& buffers, error_code& ec)
{
	if (body_.is_file())
		return file_reader_.put(buffers, ec);
	else
		return text_reader_.put(buffers, ec);
}

// Called after writing is done when there's no error.
template<class TextBody, class FileBody>
void
basic_flex_body<TextBody, FileBody>::
reader::
finish(error_code& ec)
{
    // This has to be cleared before returning, to
    // indicate no error. The specification requires it.
	if (body_.is_file())
		file_reader_.finish(ec);
	else
		text_reader_.finish(ec);
}



template<class TextBody, class FileBody>
std::ostream&
operator<<(std::ostream& os,
	typename basic_flex_body<TextBody, FileBody>::value_type const& body)
{
	if (body.is_text())
	{
		os << body.text();
	}
	else
	{
		ASIO2_ASSERT(false);
	}
	return os;
}


using flex_body = basic_flex_body<http::string_body, http::file_body>;


template<typename = void>
std::ostream&
operator<<(std::ostream& os,
	typename flex_body::value_type const& body)
{
	if (body.is_text())
	{
		os << body.text();
	}
	else
	{
        ASIO2_ASSERT(false);
	}
	return os;
}

} // http
} // beast
} // bho

#endif
