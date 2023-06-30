/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
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

		rpc_header() noexcept {}
		rpc_header(char type, id_type id, std::string_view name)
			: type_(type), id_(id), name_(name) {}
		~rpc_header() = default;

		rpc_header(const rpc_header& r) : type_(r.type_), id_(r.id_), name_(r.name_) {}
		rpc_header(rpc_header&& r) noexcept : type_(r.type_), id_(r.id_), name_(std::move(r.name_)) {}

		inline rpc_header& operator=(const rpc_header& r)
		{
			type_ = r.type_;
			id_ = r.id_;
			name_ = r.name_;
			return (*this);
		}
		inline rpc_header& operator=(rpc_header&& r) noexcept
		{
			type_ = r.type_;
			id_ = r.id_;
			name_ = std::move(r.name_);
			return (*this);
		}

		//template <class Archive>
		//inline void serialize(Archive & ar)
		//{
		//	ar(type_, id_, name_);
		//}

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(type_, id_, name_);
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(type_, id_, name_);
		}

		inline       char             type() const noexcept { return this->type_; }
		inline       id_type          id  () const noexcept { return this->id_;   }
		inline const std::string&     name() const noexcept { return this->name_; }
		inline       char         get_type() const noexcept { return this->type_; }
		inline       id_type      get_id  () const noexcept { return this->id_;   }
		inline const std::string& get_name() const noexcept { return this->name_; }

		inline bool is_request () const noexcept { return this->type_ == rpc_type_req; }
		inline bool is_response() const noexcept { return this->type_ == rpc_type_rep; }

		inline rpc_header&     type(char type            ) noexcept { this->type_ = type; return (*this); }
		inline rpc_header&     id  (id_type id           ) noexcept { this->id_   = id  ; return (*this); }
		inline rpc_header&     name(std::string_view name)          { this->name_ = name; return (*this); }
		inline rpc_header& set_type(char type            ) noexcept { this->type_ = type; return (*this); }
		inline rpc_header& set_id  (id_type id           ) noexcept { this->id_   = id  ; return (*this); }
		inline rpc_header& set_name(std::string_view name)          { this->name_ = name; return (*this); }

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
			// if the parameters of rpc calling is reference like std::string& , must remove it's 
			//   reference to std::string
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
		rpc_request() noexcept : rpc_header() { this->type_ = rpc_type_req; }
		// can't use tp_(std::forward_as_tuple(std::forward<Args>(args)...))
		// if use tp_(std::forward_as_tuple(std::forward<Args>(args)...)),
		// when the args is nlohmann::json and under gcc 9.4.0, the json::object
		// maybe changed to json::array, like this:
		// {"name":"hello","age":10} will changed to [{"name":"hello","age":10}]
		// i don't why?
		rpc_request(std::string_view name, Args&&... args)
			: rpc_header(rpc_type_req,  0, name), tp_(std::forward<Args>(args)...) {}
		rpc_request(id_type id, std::string_view name, Args&&... args)
			: rpc_header(rpc_type_req, id, name), tp_(std::forward<Args>(args)...) {}
		~rpc_request() = default;

		rpc_request(const rpc_request& r) : rpc_header(r), tp_(r.tp_) {}
		rpc_request(rpc_request&& r) noexcept : rpc_header(std::move(r)), tp_(std::move(r.tp_)) {}

		inline rpc_request& operator=(const rpc_request& r)
		{
			static_cast<rpc_header&>(*this) = r;
			tp_ = r.tp_;
			return (*this);
		}
		inline rpc_request& operator=(rpc_request&& r) noexcept
		{
			static_cast<rpc_header&>(*this) = std::move(r);
			tp_ = std::move(r.tp_);
			return (*this);
		}

		//template <class Archive>
		//void serialize(Archive & ar)
		//{
		//	ar(cereal::base_class<rpc_header>(this));

		//	detail::for_each_tuple(tp_, [&ar](auto& elem) mutable
		//	{
		//		ar(elem);
		//	});
		//}

		template <class Archive>
		void save(Archive & ar) const
		{
			ar(cereal::base_class<rpc_header>(this));

			detail::for_each_tuple(tp_, [&ar](const auto& elem) mutable
			{
				ar << elem;
			});
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar(cereal::base_class<rpc_header>(this));

			detail::for_each_tuple(tp_, [&ar](auto& elem) mutable
			{
				ar >> elem;
			});
		}

	protected:
		std::tuple<typename result_t<Args>::type...> tp_;
	};

	template<class T>
	class rpc_response : public rpc_header
	{
	public:
		rpc_response() noexcept : rpc_header() { this->type_ = rpc_type_rep; }
		rpc_response(id_type id, std::string_view name) : rpc_header(rpc_type_rep, id, name) {}
		rpc_response(id_type id, std::string_view name, const error_code& ec, T&& ret)
			: rpc_header(rpc_type_rep, id, name), ec_(ec), ret_(std::forward<T>(ret)) {}
		~rpc_response() = default;

		rpc_response(const rpc_response& r)
			: rpc_header(r), ec_(r.ec_), ret_(r.ret_) {}
		rpc_response(rpc_response&& r) noexcept
			: rpc_header(std::move(r)), ec_(std::move(r.ec_)), ret_(std::move(r.ret_)) {}

		inline rpc_response& operator=(const rpc_response& r)
		{
			static_cast<rpc_header&>(*this) = r;
			ec_ = r.ec_;
			ret_ = r.ret_;
			return (*this);
		}
		inline rpc_response& operator=(rpc_response&& r) noexcept
		{
			static_cast<rpc_header&>(*this) = std::move(r);
			ec_ = std::move(r.ec_);
			ret_ = std::move(r.ret_);
			return (*this);
		}

		//template <class Archive>
		//void serialize(Archive & ar)
		//{
		//	ar(cereal::base_class<rpc_header>(this));
		//	ar(ec_.value());
		//	ar(ret_);
		//}

		template <class Archive>
		void save(Archive & ar) const
		{
			ar << cereal::base_class<rpc_header>(this);
			ar << ec_.value();
			ar << ret_;
		}

		template <class Archive>
		void load(Archive & ar)
		{
			ar >> cereal::base_class<rpc_header>(this);
			ar >> ec_.value();
			ar >> ret_;
		}

	protected:
		error_code ec_;
		T          ret_;
	};
}

#endif // !__ASIO2_RPC_PROTOCOL_HPP__
