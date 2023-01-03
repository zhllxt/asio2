/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_CEREAL_HPP__
#define __ASIO2_CEREAL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#if defined(_MSC_VER)
#  pragma warning(disable:4389) // Signed / unsigned mismatch
#  pragma warning(disable:4018) // Signed / unsigned mismatch
#endif

#include <sstream>

#ifndef CEREAL_RAPIDJSON_NAMESPACE
#define CEREAL_RAPIDJSON_NAMESPACE cereal::rapidjson
#endif
#ifndef CEREAL_RAPIDJSON_NAMESPACE_BEGIN
#define CEREAL_RAPIDJSON_NAMESPACE_BEGIN namespace CEREAL_RAPIDJSON_NAMESPACE {
#endif
#ifndef CEREAL_RAPIDJSON_NAMESPACE_END
#define CEREAL_RAPIDJSON_NAMESPACE_END }
#endif

#include <cereal/cereal.hpp>

#include <cereal/types/array.hpp>
#include <cereal/types/atomic.hpp>
#include <cereal/types/bitset.hpp>
#include <cereal/types/chrono.hpp>
#include <cereal/types/complex.hpp>
#include <cereal/types/deque.hpp>
#include <cereal/types/forward_list.hpp>
#include <cereal/types/functional.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/queue.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/stack.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/valarray.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/portable_binary.hpp>

namespace cereal
{
	using binary_oarchive  = BinaryOutputArchive;
	using binary_iarchive  = BinaryInputArchive;
	using json_oarchive    = JSONOutputArchive;
	using json_iarchive    = JSONInputArchive;
	using xml_oarchive     = XMLOutputArchive;
	using xml_iarchive     = XMLInputArchive;
	using pbinary_oarchive = PortableBinaryOutputArchive;
	using pbinary_iarchive = PortableBinaryInputArchive;
	using exception        = Exception;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_CEREAL_HPP__
