/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <asio2/base/detail/ecs.hpp>

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
		 * @brief constructor
		 */
		rpc_recv_op() noexcept {}

		/**
		 * @brief destructor
		 */
		~rpc_recv_op() = default;

	protected:
		inline void _rpc_handle_failed_request(
			std::shared_ptr<derived_t>& this_ptr, rpc::error e, rpc_serializer& sr, rpc_header& head)
		{
			ASIO2_ASSERT(static_cast<derived_t&>(*this).io_->running_in_this_thread());

			if (head.id() != static_cast<rpc_header::id_type>(0))
			{
				derived_t& derive = static_cast<derived_t&>(*this);

				error_code ec = rpc::make_error_code(e);

				sr.reset();
				sr << head;
				sr << ec;

				derive.internal_async_send(this_ptr, sr.str());
			}
		}

		template<typename C>
		void _rpc_handle_recv(
			std::shared_ptr<derived_t>& this_ptr, std::shared_ptr<ecs_t<C>>& ecs, std::string_view data)
		{
			detail::ignore_unused(ecs);

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
				derive._do_disconnect(rpc::make_error_code(rpc::error::illegal_data), this_ptr);
				return;
			}
			// bug fixed : illegal data being parsed into string object fails to allocate
			// memory due to excessively long data
			catch (std::bad_alloc const&)
			{
				derive._do_disconnect(rpc::make_error_code(rpc::error::illegal_data), this_ptr);
				return;
			}
			catch (std::exception const&)
			{
				derive._do_disconnect(rpc::make_error_code(rpc::error::unspecified_error), this_ptr);
				return;
			}

			if /**/ (head.is_request())
			{
				head.type(rpc_type_rep);
				sr.reset();
				sr << head;
				auto fn = derive._invoker().find(head.name());
				if (fn)
				{
					// async - return true, sync - return false
					// call this function will deserialize data, so it maybe throw some exception,
					// and it will call user function inner, the user function maybe throw some 
					// exception also.
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					try
					{
				#endif
						if ((*fn)(this_ptr, std::addressof(derive), sr, dr))
							return;
				#if !defined(ASIO_NO_EXCEPTIONS) && !defined(BOOST_ASIO_NO_EXCEPTIONS)
					}
					catch (cereal::exception const&)
					{
						derive._rpc_handle_failed_request(this_ptr, rpc::error::invalid_argument, sr, head);
						return;
					}
					catch (system_error      const&)
					{
						derive._rpc_handle_failed_request(this_ptr, rpc::error::unspecified_error, sr, head);
						return;
					}
					catch (std::exception    const&)
					{
						derive._rpc_handle_failed_request(this_ptr, rpc::error::unspecified_error, sr, head);
						return;
					}
				#endif

					// The number of parameters passed in when calling rpc function exceeds 
					// the number of parameters of local function
					if (head.id() != static_cast<rpc_header::id_type>(0))
					{
						if (dr.buffer().in_avail() == 0)
						{
							derive.internal_async_send(this_ptr, sr.str());
						}
						else
						{
							derive._rpc_handle_failed_request(this_ptr, rpc::error::invalid_argument, sr, head);
						}
					}
				}
				else
				{
					if (head.id() != static_cast<rpc_header::id_type>(0))
					{
						error_code ec = rpc::make_error_code(rpc::error::not_found);
						sr << ec;
						derive.internal_async_send(this_ptr, sr.str());
					}
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
				derive._do_disconnect(rpc::make_error_code(rpc::error::no_data), this_ptr);
			}
		}

	protected:
	};
}

#endif // !__ASIO2_RPC_RECV_OP_HPP__
