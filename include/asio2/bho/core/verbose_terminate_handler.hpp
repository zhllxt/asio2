#ifndef BHO_CORE_VERBOSE_TERMINATE_HANDLER_HPP_INCLUDED
#define BHO_CORE_VERBOSE_TERMINATE_HANDLER_HPP_INCLUDED

// MS compatible compilers support #pragma once

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

//  Copyright 2022 Peter Dimov
//  Distributed under the Boost Software License, Version 1.0.
//  https://www.boost.org/LICENSE_1_0.txt

#include <asio2/bho/core/demangle.hpp>
#include <asio2/bho/throw_exception.hpp>
#include <asio2/bho/config.hpp>
#include <exception>
#include <typeinfo>
#include <cstdlib>
#include <cstdio>

namespace bho
{
namespace core
{

BHO_NORETURN inline void verbose_terminate_handler()
{
    std::set_terminate( 0 );

#if defined(BHO_NO_EXCEPTIONS)

    std::fputs( "std::terminate called with exceptions disabled\n", stderr );

#else

    try
    {
        throw;
    }
    catch( std::exception const& x )
    {
#if defined(BHO_NO_RTTI)

        char const * typeid_name = "unknown (RTTI is disabled)";

#else

        char const * typeid_name = typeid( x ).name();

        bho::core::scoped_demangled_name typeid_demangled_name( typeid_name );

        if( typeid_demangled_name.get() != 0 )
        {
            typeid_name = typeid_demangled_name.get();
        }

#endif

        bho::source_location loc = bho::get_throw_location( x );

        std::fprintf( stderr,
            "std::terminate called after throwing an exception:\n\n"
            "      type: %s\n"
            "    what(): %s\n"
            "  location: %s:%lu:%lu in function '%s'\n",

            typeid_name,
            x.what(),
            loc.file_name(), static_cast<unsigned long>( loc.line() ),
            static_cast<unsigned long>( loc.column() ), loc.function_name()
        );
    }
    catch( ... )
    {
        std::fputs( "std::terminate called after throwing an unknown exception\n", stderr );
    }

#endif

    std::fflush( stdout );
    std::abort();
}

} // namespace core
} // namespace bho

#endif  // #ifndef BHO_CORE_VERBOSE_TERMINATE_HANDLER_HPP_INCLUDED
