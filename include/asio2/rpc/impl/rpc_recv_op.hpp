/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_RECV_OP_HPP__
#define __ASIO2_RPC_RECV_OP_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <memory>
#include <future>
#include <utility>
#include <string_view>

#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>
#include <asio2/rpc/detail/rpc_protocol.hpp>
#include <asio2/rpc/detail/rpc_invoker.hpp>

namespace asio2::detail
{
	template<class derived_t, class args_t>
	class rpc_recv_op
	{
	public:
		/**
		 * @constructor
		 */
		rpc_recv_op() noexcept {}

		/**
		 * @destructor
		 */
		~rpc_recv_op() = default;

	protected:
		template<typename MatchCondition>
		inline void _rpc_handle_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view data,
			condition_wrap<MatchCondition>& condition)
		{
			detail::ignore_unused(condition);

			derived_t& derive = static_cast<derived_t&>(*this);

			ASIO2_ASSERT(derive.is_started());

			rpc_serializer   & sr   = derive.serializer_;
			rpc_deserializer & dr   = derive.deserializer_;
			rpc_header       & head = derive.header_;

			try
			{
				dr.reset(data);
				dr >> head;
			}
			catch (cereal::exception const&)
			{
				error_code ec = rpc::make_error_code(rpc::error::illegal_data);
				set_last_error(ec);
				derive._do_disconnect(ec, this_ptr);
				return;
			}
			// bug fixed : illegal data being parsed into string object fails to allocate
			// memory due to excessively long data
			catch (std::bad_alloc const&)
			{
				error_code ec = rpc::make_error_code(rpc::error::illegal_data);
				set_last_error(ec);
				derive._do_disconnect(ec, this_ptr);
				return;
			}
			catch (std::exception const&)
			{
				error_code ec = rpc::make_error_code(rpc::error::unspecified_error);
				set_last_error(ec);
				derive._do_disconnect(ec, this_ptr);
				return;
			}

			if /**/ (head.is_request())
			{
				try
				{
					head.type(rpc_type_rep);
					sr.reset();
					sr << head;
					auto* fn = derive._invoker().find(head.name());
					if (fn)
					{
						// async - return true, sync - return false
						// call this function will deserialize data, so it maybe throw some exception,
						// and it will call user function inner, the user function maybe throw some 
						// exception also.
						if ((*fn)(this_ptr, std::addressof(derive), sr, dr))
							return;

						// The number of parameters passed in when calling rpc function exceeds 
						// the number of parameters of local function
						if (dr.buffer().in_avail() != 0 && head.id() != static_cast<rpc_header::id_type>(0))
						{
							sr.reset();
							sr << head;
							error_code ec = rpc::make_error_code(rpc::error::invalid_argument);
							sr << ec;
						}
					}
					else
					{
						if (head.id() != static_cast<rpc_header::id_type>(0))
						{
							error_code ec = rpc::make_error_code(rpc::error::not_found);
							sr << ec;
						}
					}
				}
				catch (cereal::exception const&)
				{
					error_code ec = rpc::make_error_code(rpc::error::invalid_argument);
					sr << ec;
				}
				catch (system_error      const&)
				{
					error_code ec = rpc::make_error_code(rpc::error::unspecified_error);
					sr << ec;
				}
				catch (std::exception    const&)
				{
					error_code ec = rpc::make_error_code(rpc::error::unspecified_error);
					sr << ec;
				}

				if (head.id() != static_cast<rpc_header::id_type>(0))
				{
					derive.async_send(sr.str());
				}
			}
			else if (head.is_response())
			{
				auto iter = derive.reqs_.find(head.id());
				if (iter != derive.reqs_.end())
				{
					std::function<void(error_code, std::string_view)>& cb = iter->second;
					cb(rpc::make_error_code(rpc::error::success), data);
				}
			}
			else
			{
				error_code ec = rpc::make_error_code(rpc::error::no_data);
				set_last_error(ec);
				derive._do_disconnect(ec, this_ptr);
			}
		}

	protected:
	};
}

#endif // !__ASIO2_RPC_RECV_OP_HPP__
