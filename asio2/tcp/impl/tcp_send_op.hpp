/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 */

#ifndef __ASIO2_TCP_SEND_OP_HPP__
#define __ASIO2_TCP_SEND_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

namespace asio2::detail
{
	template<typename T>
	struct has_member_dgram
	{
		typedef char(&yes)[1];
		typedef char(&no)[2];

		// this creates an ambiguous &Derived::dgram_ if T has got member dgram_

		struct Fallback { char dgram_; };
		struct Derived : T, Fallback { };

		template<typename U, U>
		struct Check;

		template<typename U>
		static no test(Check<char Fallback::*, &U::dgram_>*);

		template<typename U>
		static yes test(...);

		static constexpr bool value = sizeof(test<Derived>(0)) == sizeof(yes);
	};


	template<class derived_t, bool isSession>
	class tcp_send_op
	{
	public:
		/**
		 * @constructor
		 */
		tcp_send_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~tcp_send_op() = default;

	protected:
		template<class ConstBufferSequence>
		inline bool _tcp_send(ConstBufferSequence buffer)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				return false;
			}

			error_code ec;

			if constexpr (has_member_dgram<derived_t>::value)
			{
				if (derive.dgram_)
				{
					std::size_t sent_bytes = dgram_send_head_op()(derive.stream(), buffer, ec);
					set_last_error(ec);
					if (ec)
					{
						// must stop, otherwise re-sending will cause header confusion
						if (sent_bytes > 0)
							derive._do_stop(ec);
					}
				}
			}
			else
				std::ignore = true;

			if (!ec)
			{
				asio::write(derive.stream(), buffer, ec);
				set_last_error(ec);
			}

			return (!ec.operator bool());
		}

		template<class ConstBufferSequence, class Callback>
		inline bool _tcp_send(ConstBufferSequence buffer, Callback& fn)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				callback_helper::call(fn, 0);
				return false;
			}

			error_code ec;

			if constexpr (has_member_dgram<derived_t>::value)
			{
				if (derive.dgram_)
				{
					std::size_t sent_bytes = dgram_send_head_op()(derive.stream(), buffer, ec);
					set_last_error(ec);
					if (ec)
					{
						callback_helper::call(fn, 0);
						// must stop, otherwise re-sending will cause header confusion
						if (sent_bytes > 0)
							derive._do_stop(ec);
					}
				}
			}
			else
				std::ignore = true;

			if (!ec)
			{
				std::size_t sent_bytes = asio::write(derive.stream(), buffer, ec);
				set_last_error(ec);
				callback_helper::call(fn, sent_bytes);
			}

			return (!ec.operator bool());
		}

		template<class ConstBufferSequence>
		inline bool _tcp_send(ConstBufferSequence buffer, std::promise<std::pair<error_code, std::size_t>>& promise)
		{
			if (!derive.is_started())
			{
				set_last_error(asio::error::not_connected);
				promise.set_value(std::pair<error_code, std::size_t>(asio::error::not_connected, 0));
				return false;
			}

			error_code ec;

			if constexpr (has_member_dgram<derived_t>::value)
			{
				if (derive.dgram_)
				{
					std::size_t sent_bytes = dgram_send_head_op()(derive.stream(), buffer, ec);
					set_last_error(ec);
					if (ec)
					{
						// must stop, otherwise re-sending will cause header confusion
						if (sent_bytes > 0)
							derive._do_stop(ec);
					}
				}
			}
			else
				std::ignore = true;

			if (!ec)
			{
				std::size_t sent_bytes = asio::write(derive.stream(), buffer, ec);
				set_last_error(ec);
				promise.set_value(std::pair<error_code, std::size_t>(ec, sent_bytes));
			}
			else
				promise.set_value(std::pair<error_code, std::size_t>(ec, 0));

			return (!ec.operator bool());
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_TCP_SEND_OP_HPP__
