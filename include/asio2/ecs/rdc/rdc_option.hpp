/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RDC_OPTION_HPP__
#define __ASIO2_RDC_OPTION_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>

#include <memory>
#include <functional>
#include <type_traits>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>

#include <asio2/ecs/rdc/rdc_invoker.hpp>

namespace asio2::rdc
{
	template<class IdT, class SendDataT, class RecvDataT = SendDataT>
	class option
	{
	protected:
		using send_parser_fun = std::function<IdT(SendDataT)>;
		using recv_parser_fun = std::function<IdT(RecvDataT)>;

		send_parser_fun rdc_send_parser_;
		recv_parser_fun rdc_recv_parser_;

		asio2::detail::rdc_invoker_t<IdT, SendDataT, RecvDataT> rdc_invoker_;

	public:
		option(option&&) noexcept = default;
		option(option const&) = default;
		option& operator=(option&&) noexcept = default;
		option& operator=(option const&) = default;

		template<class ParserFun, std::enable_if_t<
			!std::is_base_of_v<option, detail::remove_cvref_t<ParserFun>>, int> = 0>
		explicit option(ParserFun&& parser)
			: rdc_send_parser_(std::forward<ParserFun>(parser))
			, rdc_recv_parser_(rdc_send_parser_)
		{
		}
		template<class SendParserFun, class RecvParserFun>
		explicit option(SendParserFun&& send_parser, RecvParserFun&& recv_parser)
			: rdc_send_parser_(std::forward<SendParserFun>(send_parser))
			, rdc_recv_parser_(std::forward<RecvParserFun>(recv_parser))
		{
		}

		template<class ParserFun>
		option& set_send_parser(ParserFun&& parser)
		{
			rdc_send_parser_ = std::forward<ParserFun>(parser);
			return (*this);
		}
		template<class ParserFun>
		option& set_recv_parser(ParserFun&& parser)
		{
			rdc_recv_parser_ = std::forward<ParserFun>(parser);
			return (*this);
		}

		send_parser_fun& get_send_parser() noexcept { return rdc_send_parser_; }
		recv_parser_fun& get_recv_parser() noexcept { return rdc_recv_parser_; }

		asio2::detail::rdc_invoker_t<IdT, SendDataT, RecvDataT>& invoker() noexcept { return rdc_invoker_; }
	};

	// C++17 class template argument deduction guides
	template<class ParserFun>
	option(ParserFun)->option<
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<ParserFun>>::return_type,
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<ParserFun>>::template args<0>::type,
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<ParserFun>>::template args<0>::type
	>;

	template<class SendParserFun, class RecvParserFun>
	option(SendParserFun, RecvParserFun)->option<
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<SendParserFun>>::return_type,
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<SendParserFun>>::template args<0>::type,
		typename asio2::detail::function_traits<asio2::detail::remove_cvref_t<RecvParserFun>>::template args<0>::type
	>;
}

#endif // !__ASIO2_RDC_OPTION_HPP__
