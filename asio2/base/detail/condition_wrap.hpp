/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_CONDITION_WRAP_HPP__
#define __ASIO2_CONDITION_WRAP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <string_view>
#include <regex>
#include <map>
#include <memory>

#include <asio2/base/detail/rdc_invoker.hpp>

namespace asio2::detail
{
	namespace
	{
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		using diff_type = typename iterator::difference_type;
		std::pair<iterator, bool> dgram_match_role(iterator begin, iterator end)
		{
			iterator i = begin;
			while (i != end)
			{
				// If 0~254, current byte are the payload length.
				if (std::uint8_t(*i) < std::uint8_t(254))
				{
					std::uint8_t payload_size = std::uint8_t(*i);

					++i;

					if (end - i < static_cast<diff_type>(payload_size))
						break;

					return std::pair(i + static_cast<diff_type>(payload_size), true);
				}

				// If 254, the following 2 bytes interpreted as a 16-bit unsigned integer
				// are the payload length.
				if (std::uint8_t(*i) == std::uint8_t(254))
				{
					++i;

					if (end - i < 2)
						break;

					std::uint16_t payload_size = *(reinterpret_cast<const std::uint16_t*>(i.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(&payload_size));
					}

					i += 2;
					if (end - i < static_cast<diff_type>(payload_size))
						break;

					return std::pair(i + static_cast<diff_type>(payload_size), true);
				}

				// If 255, the following 8 bytes interpreted as a 64-bit unsigned integer
				// (the most significant bit MUST be 0) are the payload length.
				if (std::uint8_t(*i) == 255)
				{
					++i;

					if (end - i < 8)
						break;

					std::uint64_t payload_size = *(reinterpret_cast<const std::uint64_t*>(i.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint64_t)>(reinterpret_cast<std::uint8_t*>(&payload_size));
					}

					i += 8;
					if (end - i < static_cast<diff_type>(payload_size))
						break;

					return std::pair(i + static_cast<diff_type>(payload_size), true);
				}

				ASIO2_ASSERT(false);
			}
			return std::pair(begin, false);
		}
	}
}

namespace asio2::detail
{
	//struct use_sync_t {};
	struct use_kcp_t {};
	struct use_dgram_t {};
	struct hook_buffer_t {};
}

namespace asio2::detail
{
	template<class T>
	class condition_traits
	{
	public:
		using type = T;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits(T c) : condition_(std::move(c)) {}
		inline T & operator()() { return this->condition_; }
	protected:
		T condition_;
	};

	template<>
	class condition_traits<void>
	{
	public:
		using type = void;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits() = default;
	};

	template<>
	class condition_traits<char>
	{
	public:
		using type = char;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits(char c) : condition_(c) {}
		inline char operator()() { return this->condition_; }
	protected:
		char condition_ = '\0';
	};

	//template<>
	//class condition_traits<std::string>
	//{
	//public:
	//	using type = std::string;
	//	condition_traits(const std::string & c) : condition_(c) {}
	//	condition_traits(std::string && c) : condition_(std::move(c)) {}
	//protected:
	//	std::string condition_;
	//};

	//template<>
	//class condition_traits<std::string_view>
	//{
	//public:
	//	using type = std::string_view;
	//	condition_traits(std::string_view c) : condition_(c) {}
	//protected:
	//	std::string_view condition_;
	//};

	//template<>
	//class condition_traits<std::regex>
	//{
	//public:
	//	using type = std::regex;
	//	condition_traits(const std::regex & c) : condition_(c) {}
	//	condition_traits(std::regex && c) : condition_(std::move(c)) {}
	//protected:
	//	std::regex condition_;
	//};

	template<>
	class condition_traits<use_dgram_t>
	{
	public:
		using type = use_dgram_t;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits(use_dgram_t) {}
		inline auto& operator()() { return dgram_match_role; }
	protected:
	};

	template<>
	class condition_traits<use_kcp_t>
	{
	public:
		using type = use_kcp_t;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits(use_kcp_t) {}
		inline asio::detail::transfer_at_least_t operator()() { return asio::transfer_at_least(1); }
	protected:
	};

	template<>
	class condition_traits<hook_buffer_t>
	{
	public:
		using type = hook_buffer_t;

		condition_traits(condition_traits&&) = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) = default;
		condition_traits& operator=(condition_traits const&) = default;

		condition_traits(hook_buffer_t) {}
		inline asio::detail::transfer_at_least_t operator()() { return asio::transfer_at_least(1); }
	protected:
	};
}

namespace asio2::detail
{
	// rdc : remote data call
	template<class ConditionT, class IdT, class SendDataT, class RecvDataT>
	struct use_rdc_t
	{
		using type = ConditionT;
		using send_parser_fun = std::function<IdT(SendDataT)>;
		using recv_parser_fun = std::function<IdT(RecvDataT)>;

		use_rdc_t(use_rdc_t&&) = default;
		use_rdc_t(use_rdc_t const&) = delete;
		use_rdc_t& operator=(use_rdc_t&&) = default;
		use_rdc_t& operator=(use_rdc_t const&) = delete;

		template<class ParserFun>
		use_rdc_t(ConditionT c, ParserFun&& parser)
			: condition_  (std::move(c))
			, send_parser_(std::forward<ParserFun>(parser))
			, recv_parser_(send_parser_)
		{
		}
		template<class SendParserFun, class RecvParserFun>
		use_rdc_t(ConditionT c, SendParserFun&& send_parser, RecvParserFun&& recv_parser)
			: condition_  (std::move(c))
			, send_parser_(std::forward<SendParserFun>(send_parser))
			, recv_parser_(std::forward<RecvParserFun>(recv_parser))
		{
		}
		inline decltype(auto)    operator()() { return condition_(); }
		inline send_parser_fun& send_parser() { return send_parser_; }
		inline recv_parser_fun& recv_parser() { return recv_parser_; }
		inline rdc_invoker_t<IdT, SendDataT, RecvDataT>& invoker() { return invoker_; }
	protected:
		condition_traits<ConditionT>                    condition_;
		send_parser_fun                                 send_parser_;
		recv_parser_fun                                 recv_parser_;
		rdc_invoker_t<IdT, SendDataT, RecvDataT>        invoker_;
	};

	template<class IdT, class SendDataT, class RecvDataT>
	struct use_rdc_t<void, IdT, SendDataT, RecvDataT>
	{
		using type = void;
		using send_parser_fun = std::function<IdT(SendDataT)>;
		using recv_parser_fun = std::function<IdT(RecvDataT)>;

		use_rdc_t(use_rdc_t&&) = default;
		use_rdc_t(use_rdc_t const&) = delete;
		use_rdc_t& operator=(use_rdc_t&&) = default;
		use_rdc_t& operator=(use_rdc_t const&) = delete;

		template<class ParserFun>
		use_rdc_t(ParserFun&& parser)
			: send_parser_(std::forward<ParserFun>(parser))
			, recv_parser_(send_parser_)
		{
		}
		template<class SendParserFun, class RecvParserFun>
		use_rdc_t(SendParserFun&& send_parser, RecvParserFun&& recv_parser)
			: send_parser_(std::forward<SendParserFun>(send_parser))
			, recv_parser_(std::forward<RecvParserFun>(recv_parser))
		{
		}
		inline send_parser_fun& send_parser() { return send_parser_; }
		inline recv_parser_fun& recv_parser() { return recv_parser_; }
		inline rdc_invoker_t<IdT, SendDataT, RecvDataT>& invoker() { return invoker_; }
	protected:
		condition_traits<void>                          condition_;
		send_parser_fun                                 send_parser_;
		recv_parser_fun                                 recv_parser_;
		rdc_invoker_t<IdT, SendDataT, RecvDataT>        invoker_;
	};
}

namespace asio2::detail
{
	template<class T>
	class condition_wrap
	{
	public:
		using traits_type = condition_traits<T>;
		using condition_type = typename traits_type::type;

		condition_wrap(condition_wrap&&) = default;
		condition_wrap(condition_wrap const&) = default;
		condition_wrap& operator=(condition_wrap&&) = default;
		condition_wrap& operator=(condition_wrap const&) = default;

		condition_wrap(T c) : condition_(std::move(c)) {}
		inline decltype(auto) operator()() { return condition_(); }
	protected:
		traits_type condition_;
	};

	template<>
	class condition_wrap<void>
	{
	public:
		using type = void;
		using condition_type = void;

		condition_wrap(condition_wrap&&) = default;
		condition_wrap(condition_wrap const&) = default;
		condition_wrap& operator=(condition_wrap&&) = default;
		condition_wrap& operator=(condition_wrap const&) = default;

		condition_wrap() = default;
	};

	template<class ConditionT, class IdT, class SendDataT, class RecvDataT>
	class condition_wrap<use_rdc_t<ConditionT, IdT, SendDataT, RecvDataT>>
	{
	public:
		using traits_type = use_rdc_t<ConditionT, IdT, SendDataT, RecvDataT>;
		using condition_type = typename traits_type::type;
		using send_parser_fun = typename traits_type::send_parser_fun;
		using recv_parser_fun = typename traits_type::recv_parser_fun;

		condition_wrap(condition_wrap&&) = default;
		condition_wrap(condition_wrap const&) = default;
		condition_wrap& operator=(condition_wrap&&) = default;
		condition_wrap& operator=(condition_wrap const&) = default;

		template<class... Args>
		condition_wrap(std::in_place_t, Args&&... args)
		{
			condition_ = std::make_shared<traits_type>(std::forward<Args>(args)...);
		}

		inline decltype(auto)    operator()() { return (*condition_)(); }
		inline send_parser_fun& send_parser() { return condition_->send_parser(); }
		inline recv_parser_fun& recv_parser() { return condition_->recv_parser(); }
		inline rdc_invoker_t<IdT, SendDataT, RecvDataT>& invoker() { return condition_->invoker(); }
	protected:
		std::shared_ptr<traits_type> condition_;
	};
}

namespace asio2
{
	//constexpr static detail::use_sync_t    use_sync;

	// https://github.com/skywind3000/kcp
	constexpr static detail::use_kcp_t     use_kcp;

	constexpr static detail::use_dgram_t   use_dgram;

	constexpr static detail::hook_buffer_t hook_buffer;
}

#endif // !__ASIO2_CONDITION_WRAP_HPP__
