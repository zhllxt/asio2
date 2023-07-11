/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <any>

#include <asio2/base/error.hpp>
#include <asio2/base/define.hpp>

#include <asio2/base/detail/util.hpp>
#include <asio2/base/detail/function_traits.hpp>

#include <asio2/component/rdc/rdc_invoker.hpp>

namespace asio2::rdc
{
	struct option_base
	{
		virtual ~option_base() {}
		virtual std::any call_parser(bool is_send, void* data) = 0;
		virtual void emplace_request(std::any& k, void* v) = 0;
		virtual void execute_and_erase(std::any& k, std::function<void(void*)> cb) = 0;
		virtual void foreach_and_clear(std::function<void(void*, void*)> cb) = 0;
	};

	template<class IdT, class SendDataT, class RecvDataT = SendDataT>
	class option : public rdc::option_base
	{
	protected:
		using send_parser_fun = std::function<IdT(SendDataT)>;
		using recv_parser_fun = std::function<IdT(RecvDataT)>;

		send_parser_fun rdc_send_parser_;
		recv_parser_fun rdc_recv_parser_;

		using invoker_type  = detail::rdc_invoker_t<IdT, SendDataT, RecvDataT>;
		using iterator_type = typename invoker_type::iterator_type;

		using key_type = IdT;
		using val_type = typename invoker_type::value_type;

		invoker_type rdc_invoker_;

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

		detail::rdc_invoker_t<IdT, SendDataT, RecvDataT>& invoker() noexcept { return rdc_invoker_; }

		virtual std::any call_parser(bool is_send, void* data) override
		{
			if (is_send)
			{
				typename detail::remove_cvref_t<SendDataT>& d = *((typename detail::remove_cvref_t<SendDataT>*)data);
				key_type id = rdc_send_parser_(d);
				return std::any(std::move(id));
			}
			else
			{
				typename detail::remove_cvref_t<RecvDataT>& d = *((typename detail::remove_cvref_t<RecvDataT>*)data);
				key_type id = rdc_recv_parser_(d);
				return std::any(std::move(id));
			}
		}
		virtual void emplace_request(std::any& k, void* v) override
		{
			key_type& key = *std::any_cast<key_type>(std::addressof(k));
			val_type& val = *((val_type*)v);

			// 2023-07-11 bug fix : can't use std::move(key), beacuse the key will be used at later.
			rdc_invoker_.emplace(key, std::move(val));
		}
		virtual void execute_and_erase(std::any& k, std::function<void(void*)> cb) override
		{
			key_type& key = *std::any_cast<key_type>(std::addressof(k));

			if (auto iter = rdc_invoker_.find(key); iter != rdc_invoker_.end())
			{
				cb((void*)(std::addressof(iter->second)));

				rdc_invoker_.erase(iter);
			}
		}
		virtual void foreach_and_clear(std::function<void(void*, void*)> cb) override
		{
			for (auto& [k, v] : rdc_invoker_.reqs())
			{
				cb((void*)std::addressof(k), (void*)std::addressof(v));
			}

			rdc_invoker_.reqs().clear();
		}
	};

	// C++17 class template argument deduction guides
	template<class ParserFun>
	option(ParserFun)->option<
		typename detail::function_traits<detail::remove_cvref_t<ParserFun>>::return_type,
		typename detail::function_traits<detail::remove_cvref_t<ParserFun>>::template args<0>::type,
		typename detail::function_traits<detail::remove_cvref_t<ParserFun>>::template args<0>::type
	>;

	template<class SendParserFun, class RecvParserFun>
	option(SendParserFun, RecvParserFun)->option<
		typename detail::function_traits<detail::remove_cvref_t<SendParserFun>>::return_type,
		typename detail::function_traits<detail::remove_cvref_t<SendParserFun>>::template args<0>::type,
		typename detail::function_traits<detail::remove_cvref_t<RecvParserFun>>::template args<0>::type
	>;
}

#endif // !__ASIO2_RDC_OPTION_HPP__
