/*
 * COPYRIGHT (C) 2017-2019, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_RPC_PROTOCOL_HPP__
#define __ASIO2_RPC_PROTOCOL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <string>
#include <string_view>

#include <asio2/base/detail/util.hpp>

#include <asio2/rpc/detail/rpc_serialization.hpp>

namespace asio2::detail
{
	struct use_tcp {};
	struct use_websocket {};


	/*
	 * request  : message type + request id + function name + parameters value...
	 * response : message type + request id + function name + error code + result value
	 *
	 * message type : q - request, p - response
	 *
	 * if result type is void, then result type will wrapped to std::int8_t
	 */

	static constexpr char rpc_type_req = 'q';
	static constexpr char rpc_type_rep = 'p';

	class rpc_header
	{
	public:
		using id_type = std::uint64_t;

		rpc_header() {}
		rpc_header(char type, id_type id, std::string_view name)
			: type_(type), id_(id), name_(name) {}
		~rpc_header() = default;

		rpc_header(const rpc_header& r) : type_(r.type_), id_(r.id_), name_(r.name_) {}
		rpc_header(rpc_header&& r) : type_(r.type_), id_(r.id_), name_(std::move(r.name_)) {}

		inline rpc_header& operator=(const rpc_header& r)
		{
			type_ = r.type_;
			id_ = r.id_;
			name_ = r.name_;
			return (*this);
		}
		inline rpc_header& operator=(rpc_header&& r)
		{
			type_ = r.type_;
			id_ = r.id_;
			name_ = std::move(r.name_);
			return (*this);
		}

		template <class Archive>
		inline void serialize(Archive & ar)
		{
			ar(type_, id_, name_);
		}

		inline const char         type() const { return this->type_; }
		inline const id_type      id()   const { return this->id_;   }
		inline const std::string& name() const { return this->name_; }

		inline bool is_request()  { return this->type_ == rpc_type_req; }
		inline bool is_response() { return this->type_ == rpc_type_rep; }

		inline rpc_header& type(char type            ) { this->type_ = type; return (*this); }
		inline rpc_header& id  (id_type id           ) { this->id_   = id  ; return (*this); }
		inline rpc_header& name(std::string_view name) { this->name_ = name; return (*this); }

	protected:
		char           type_;
		id_type        id_ = 0;
		std::string    name_;
	};

	template<class ...Args>
	class rpc_request : public rpc_header
	{
	protected:
		template<class T>
		struct value_t
		{
			using type = T;
		};
		template<class... Ts>
		struct value_t<std::basic_string_view<Ts...>>
		{
			using type = typename std::basic_string_view<Ts...>::value_type;
		};

		template<class T>
		struct result_t
		{
			// if the parameters of rpc calling is raw pointer like char* , must convert it to std::string
			// if the parameters of rpc calling is std::string_view , must convert it to std::string
			// if the parameters of rpc calling is refrence like std::string& , must remove it's 
			//   refrence to std::string
			using ncvr_type = std::remove_cv_t<std::remove_reference_t<T>>;
			using char_type = std::remove_cv_t<std::remove_reference_t<
				std::remove_all_extents_t<std::remove_pointer_t<ncvr_type>>>>;
			using type = std::conditional_t<
				(std::is_pointer_v<ncvr_type> || std::is_array_v<ncvr_type>) && (
					std::is_same_v<char_type, std::string::value_type> ||
					std::is_same_v<char_type, std::wstring::value_type> ||
					std::is_same_v<char_type, std::u16string::value_type> ||
					std::is_same_v<char_type, std::u32string::value_type>)
				, std::basic_string<char_type>
				, std::conditional_t<is_template_instance_of_v<std::basic_string_view, ncvr_type>
				, std::basic_string<typename value_t<ncvr_type>::type>
				, ncvr_type>>;
		};

	public:
		rpc_request() : rpc_header() { this->type_ = rpc_type_req; }
		rpc_request(std::string_view name, Args&&... args)
			: rpc_header(rpc_type_req,  0, name), tp_(std::forward_as_tuple(std::forward<Args>(args)...)) {}
		rpc_request(id_type id, std::string_view name, Args&&... args)
			: rpc_header(rpc_type_req, id, name), tp_(std::forward_as_tuple(std::forward<Args>(args)...)) {}
		~rpc_request() = default;

		rpc_request(const rpc_request& r) : rpc_header(r), tp_(r.tp_) {}
		rpc_request(rpc_request&& r) : rpc_header(std::move(r)), tp_(std::move(r.tp_)) {}

		inline rpc_request& operator=(const rpc_request& r)
		{
			static_cast<rpc_header&>(*this) = r;
			tp_ = r.tp_;
			return (*this);
		}
		inline rpc_request& operator=(rpc_request&& r)
		{
			static_cast<rpc_header&>(*this) = std::move(r);
			tp_ = std::move(r.tp_);
			return (*this);
		}

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(cereal::base_class<rpc_header>(this));
			ar(tp_);
		}

	protected:
		std::tuple<typename result_t<Args>::type...> tp_;
	};

	template<class T>
	class rpc_response : public rpc_header
	{
	public:
		rpc_response() : rpc_header() { this->type_ = rpc_type_rep; }
		rpc_response(id_type id, std::string_view name) : rpc_header(rpc_type_rep, id, name) {}
		rpc_response(id_type id, std::string_view name, const error_code& ec, T&& ret)
			: rpc_header(rpc_type_rep, id, name), ec_(ec), ret_(std::forward<T>(ret)) {}
		~rpc_response() = default;

		rpc_response(const rpc_response& r) : rpc_header(r), ec_(r.ec_), ret_(r.ret_) {}
		rpc_response(rpc_response&& r) : rpc_header(std::move(r)), ec_(std::move(r.ec_)), ret_(std::move(r.ret_)) {}

		inline rpc_response& operator=(const rpc_response& r)
		{
			static_cast<rpc_header&>(*this) = r;
			ec_ = r.ec_;
			ret_ = r.ret_;
			return (*this);
		}
		inline rpc_response& operator=(rpc_response&& r)
		{
			static_cast<rpc_header&>(*this) = std::move(r);
			ec_ = std::move(r.ec_);
			ret_ = std::move(r.ret_);
			return (*this);
		}

		template <class Archive>
		void serialize(Archive & ar)
		{
			ar(cereal::base_class<rpc_header>(this));
			ar(ec_.value());
			ar(ret_);
		}

	protected:
		error_code ec_;
		T          ret_;
	};
}

#endif // !__ASIO2_RPC_PROTOCOL_HPP__
