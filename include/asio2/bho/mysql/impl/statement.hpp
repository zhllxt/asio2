//
// Copyright (c) 2019-2023 Ruben Perez Hidalgo (rubenperez038 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BHO_MYSQL_IMPL_STATEMENT_HPP
#define BHO_MYSQL_IMPL_STATEMENT_HPP

#pragma once

#include <asio2/bho/mysql/statement.hpp>

#include <asio2/bho/mysql/detail/access.hpp>

#include <asio2/bho/assert.hpp>

template <BHO_MYSQL_WRITABLE_FIELD_TUPLE WritableFieldTuple>
class bho::mysql::bound_statement_tuple
{
    friend class statement;
    friend struct detail::access;

    struct impl
    {
        statement stmt;
        WritableFieldTuple params;
    } impl_;

    template <typename TupleType>
    bound_statement_tuple(const statement& stmt, TupleType&& t) : impl_{stmt, std::forward<TupleType>(t)}
    {
    }
};

template <BHO_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator>
class bho::mysql::bound_statement_iterator_range
{
    friend class statement;
    friend struct detail::access;

    struct impl
    {
        statement stmt;
        FieldViewFwdIterator first;
        FieldViewFwdIterator last;
    } impl_;

    bound_statement_iterator_range(
        const statement& stmt,
        FieldViewFwdIterator first,
        FieldViewFwdIterator last
    )
        : impl_{stmt, first, last}
    {
    }
};

template <BHO_MYSQL_WRITABLE_FIELD_TUPLE WritableFieldTuple, typename EnableIf>
bho::mysql::bound_statement_tuple<typename std::decay<WritableFieldTuple>::type> bho::mysql::statement::
    bind(WritableFieldTuple&& args) const

{
    BHO_ASSERT(valid());
    return bound_statement_tuple<typename std::decay<WritableFieldTuple>::type>(
        *this,
        std::forward<WritableFieldTuple>(args)
    );
}

template <BHO_MYSQL_FIELD_VIEW_FORWARD_ITERATOR FieldViewFwdIterator, typename EnableIf>
bho::mysql::bound_statement_iterator_range<FieldViewFwdIterator> bho::mysql::statement::bind(
    FieldViewFwdIterator first,
    FieldViewFwdIterator last
) const
{
    BHO_ASSERT(valid());
    return bound_statement_iterator_range<FieldViewFwdIterator>(*this, first, last);
}

#endif
