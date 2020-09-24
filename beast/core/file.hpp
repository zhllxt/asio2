//
// Copyright (c) 2015-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BEAST_CORE_FILE_HPP
#define BEAST_CORE_FILE_HPP

#include <beast/core/detail/config.hpp>
#include <beast/core/file_base.hpp>
#include <beast/core/file_stdio.hpp>

namespace beast {

/** An implementation of File.

    This alias is set to the best available implementation
    of <em>File</em> given the platform and build settings.
*/
#if BEAST_DOXYGEN
struct file : file_stdio
{
};
#else
using file = file_stdio;
#endif

} // beast

#endif
