#ifndef BHO_CONFIG_HELPER_MACROS_HPP_INCLUDED
#define BHO_CONFIG_HELPER_MACROS_HPP_INCLUDED

//  Copyright 2001 John Maddock.
//  Copyright 2017 Peter Dimov.
//
//  Distributed under the Boost Software License, Version 1.0.
//
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  BHO_STRINGIZE(X)
//  BHO_JOIN(X, Y)
//
//  Note that this header is C compatible.

//
// Helper macro BHO_STRINGIZE:
// Converts the parameter X to a string after macro replacement
// on X has been performed.
//
#define BHO_STRINGIZE(X) BHO_DO_STRINGIZE(X)
#define BHO_DO_STRINGIZE(X) #X

//
// Helper macro BHO_JOIN:
// The following piece of macro magic joins the two
// arguments together, even when one of the arguments is
// itself a macro (see 16.3.1 in C++ standard).  The key
// is that macro expansion of macro arguments does not
// occur in BHO_DO_JOIN2 but does in BHO_DO_JOIN.
//
#define BHO_JOIN(X, Y) BHO_DO_JOIN(X, Y)
#define BHO_DO_JOIN(X, Y) BHO_DO_JOIN2(X,Y)
#define BHO_DO_JOIN2(X, Y) X##Y

#endif // BHO_CONFIG_HELPER_MACROS_HPP_INCLUDED
