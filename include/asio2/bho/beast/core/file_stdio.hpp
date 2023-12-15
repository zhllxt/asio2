//
// Copyright (c) 2015-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BHO_BEAST_CORE_FILE_STDIO_HPP
#define BHO_BEAST_CORE_FILE_STDIO_HPP

#include <asio2/bho/beast/core/detail/config.hpp>
#include <asio2/bho/beast/core/error.hpp>
#include <asio2/bho/beast/core/file_base.hpp>
#include <cstdio>
#include <cstdint>
#include <fstream>

namespace bho {
namespace beast {

/** An implementation of File which uses cstdio.

    This class implements a file using the interfaces present
    in the C++ Standard Library, in `<stdio>`.
*/
class file_stdio
{
    mutable std::fstream f_;

public:
    /** The type of the underlying file handle.

        This is platform-specific.
    */
    using native_handle_type = std::fstream;

    /** Destructor

        If the file is open it is first closed.
    */
    BHO_BEAST_DECL
    ~file_stdio();

    /** Constructor

        There is no open file initially.
    */
    file_stdio() = default;

    /** Constructor

        The moved-from object behaves as if default constructed.
    */
    BHO_BEAST_DECL
    file_stdio(file_stdio&& other);

    /** Assignment

        The moved-from object behaves as if default constructed.
    */
    BHO_BEAST_DECL
    file_stdio& operator=(file_stdio&& other);

    /// Returns the native handle associated with the file.
    const std::fstream&
    native_handle() const
    {
        return f_;
    }

    /** Set the native handle associated with the file.

        If the file is open it is first closed.

        @param f The native file handle to assign.
    */
    BHO_BEAST_DECL
    void
    native_handle(std::fstream& f);

    /// Returns `true` if the file is open
    bool
    is_open() const
    {
        return f_.is_open();
    }

    /** Close the file if open

        @param ec Set to the error, if any occurred.
    */
    BHO_BEAST_DECL
    void
    close(error_code& ec);

    /** Open a file at the given path with the specified mode

        @param path The utf-8 encoded path to the file

        @param mode The file mode to use

        @param ec Set to the error, if any occurred
    */
    BHO_BEAST_DECL
    void
    open(char const* path, file_mode mode, error_code& ec);

    /** Return the size of the open file

        @param ec Set to the error, if any occurred

        @return The size in bytes
    */
    BHO_BEAST_DECL
    std::uint64_t
    size(error_code& ec) const;

    /** Return the current position in the open file

        @param ec Set to the error, if any occurred

        @return The offset in bytes from the beginning of the file
    */
    BHO_BEAST_DECL
    std::uint64_t
    pos(error_code& ec) const;

    /** Adjust the current position in the open file

        @param offset The offset in bytes from the beginning of the file

        @param ec Set to the error, if any occurred
    */
    BHO_BEAST_DECL
    void
    seek(std::uint64_t offset, error_code& ec);

    /** Read from the open file

        @param buffer The buffer for storing the result of the read

        @param n The number of bytes to read

        @param ec Set to the error, if any occurred
    */
    BHO_BEAST_DECL
    std::size_t
    read(void* buffer, std::size_t n, error_code& ec) const;

    /** Write to the open file

        @param buffer The buffer holding the data to write

        @param n The number of bytes to write

        @param ec Set to the error, if any occurred
    */
    BHO_BEAST_DECL
    std::size_t
    write(void const* buffer, std::size_t n, error_code& ec);
};

} // beast
} // bho

#ifdef BEAST_HEADER_ONLY
#include <asio2/bho/beast/core/impl/file_stdio.ipp>
#endif

#endif
