/*
 * COPYRIGHT (C) 2017-2019, zhllxt
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

#include <asio2/base/selector.hpp>
#include <asio2/base/error.hpp>
#include <asio2/base/detail/condition_wrap.hpp>

#include <asio2/rpc/detail/serialization.hpp>
#include <asio2/rpc/detail/protocol.hpp>
#include <asio2/rpc/detail/invoker.hpp>

namespace asio2::detail
{
	template<class derived_t, bool isSession>
	class rpc_recv_op
	{
	public:
		/**
		 * @constructor
		 */
		rpc_recv_op() : derive(static_cast<derived_t&>(*this)) {}

		/**
		 * @destructor
		 */
		~rpc_recv_op() = default;

	protected:
		inline void _rpc_handle_recv(std::shared_ptr<derived_t>& this_ptr, std::string_view s)
		{
			serializer& sr = derive.serializer_;
			deserializer& dr = derive.deserializer_;
			header& head = derive.header_;

			try
			{
				dr.reset(s);
				dr >> head;
			}
			catch (cereal::exception&)
			{
				set_last_error(asio::error::message_size);
				derive._do_disconnect(asio::error::message_size);
				return;
			}
			// bug fixed : illegal data being parsed into string object fails to allocate
			// memory due to excessively long data
			catch (std::bad_alloc&)
			{
				set_last_error(asio::error::message_size);
				derive._do_disconnect(asio::error::message_size);
				return;
			}
			catch (std::exception&)
			{
				set_last_error(asio::error::message_size);
				derive._do_disconnect(asio::error::message_size);
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
						(*fn)(this_ptr, sr, dr);

						// The number of parameters passed in when calling rpc function exceeds 
						// the number of parameters of local function
						if (dr.buffer().in_avail() != 0 && head.id() != static_cast<header::id_type>(0))
						{
							sr.reset();
							sr << head;
							asio::detail::throw_error(asio::error::invalid_argument);
						}
					}
					else
					{
						if (head.id() != static_cast<header::id_type>(0))
						{
							sr << error_code{ asio::error::not_found };
						}
					}
				}
				catch (cereal::exception&  ) { sr << error_code{ asio::error::no_data          }; }
				catch (system_error     & e) { sr << e.code();                                    }
				catch (std::exception   &  ) { sr << error_code{ asio::error::invalid_argument }; }

				if (head.id() != static_cast<header::id_type>(0))
				{
					const std::string& str = sr.str();
					derive.send(str);
				}
			}
			else if (head.is_response())
			{
				auto iter = derive.reqs_.find(head.id());
				if (iter == derive.reqs_.end())
					return;
				std::function<void(error_code, std::string_view)>& cb = iter->second;
				cb(error_code{}, s);
			}
			else
			{
				set_last_error(asio::error::no_data);
				derive._do_disconnect(asio::error::no_data);
			}
		}

	protected:
		derived_t & derive;
	};
}

#endif // !__ASIO2_RPC_RECV_OP_HPP__
