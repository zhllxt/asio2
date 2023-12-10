/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_HTTP_RESPONSE_IMPL_HPP__
#define __ASIO2_HTTP_RESPONSE_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <asio2/base/define.hpp>
#include <asio2/base/detail/filesystem.hpp>
#include <asio2/base/impl/user_data_cp.hpp>

#include <asio2/http/detail/flex_body.hpp>
#include <asio2/http/detail/http_util.hpp>

namespace asio2::detail
{
	template<class, class> class http_router_t;
}

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	class response_defer
	{
		template<class, class> friend class asio2::detail::http_router_t;

	public:
		response_defer(std::function<void()> cb, std::shared_ptr<void> session)
			: cb_(std::move(cb)), session_(std::move(session))
		{
			ASIO2_ASSERT(session_);
		}
		~response_defer()
		{
			if (aop_after_cb_)
			{
				(*aop_after_cb_)();
			}
			if (cb_)
			{
				cb_();
			}
		}

	protected:
		std::function<void()> cb_;

		std::unique_ptr<std::function<void()>> aop_after_cb_;

		// hold the http_session ptr, otherwise when the cb_ is calling, the session
		// maybe destroyed already, then the response("rep_") in the cb_ is destroyed
		// already, then it cause crash.
		std::shared_ptr<void> session_;
	};
}

namespace asio2::detail
{
	ASIO2_CLASS_FORWARD_DECLARE_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_BASE;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_CLIENT;
	ASIO2_CLASS_FORWARD_DECLARE_TCP_SESSION;

	template<class Body, class Fields = http::fields>
	class http_response_impl_t
		: public http::message<false, Body, Fields>
	#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
		, public user_data_cp<http_response_impl_t<Body, Fields>>
	#endif
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

		template<class, class> friend class asio2::detail::http_router_t;

	public:
		using self  = http_response_impl_t<Body, Fields>;
		using super = http::message<false, Body, Fields>;
		using header_type = typename super::header_type;
		using body_type = typename super::body_type;

	public:
		/**
		 * @brief constructor
		 * this default constructor it used for rdc call, beacuse the default status of
		 * http response is 200, and if the rdc call failed, the status of the returned
		 * http response is 200 too, so we set the status to another value at here.
		 */
		explicit http_response_impl_t()
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			super::result(http::status::unknown);
		}

		template<typename... Args>
		explicit http_response_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
		}

		http_response_impl_t(const http_response_impl_t& o)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base() = o.base();
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
		}

		http_response_impl_t(http_response_impl_t&& o)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base() = std::move(o.base());
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
		}

		self& operator=(const http_response_impl_t& o)
		{
			this->base() = o.base();
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
			return *this;
		}

		self& operator=(http_response_impl_t&& o)
		{
			this->base() = std::move(o.base());
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
			return *this;
		}

		template<class BodyT = Body>
		http_response_impl_t(const http::message<false, BodyT, Fields>& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base() = rep;
		}

		template<class BodyT = Body>
		http_response_impl_t(http::message<false, BodyT, Fields>&& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base() = std::move(rep);
		}

		template<class BodyT = Body>
		self& operator=(const http::message<false, BodyT, Fields>& rep)
		{
			this->base() = rep;
			return *this;
		}

		template<class BodyT = Body>
		self& operator=(http::message<false, BodyT, Fields>&& rep)
		{
			this->base() = std::move(rep);
			return *this;
		}

		//-------------------------------------------------

		http_response_impl_t(const http::message<false, http::string_body, Fields>& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base().base() = rep.base();
			this->body().text() = rep.body();
		}

		http_response_impl_t(http::message<false, http::string_body, Fields>&& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
		}

		self& operator=(const http::message<false, http::string_body, Fields>& rep)
		{
			this->base().base() = rep.base();
			this->body().text() = rep.body();
			return *this;
		}

		self& operator=(http::message<false, http::string_body, Fields>&& rep)
		{
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
			return *this;
		}

		//-------------------------------------------------

		http_response_impl_t(const http::message<false, http::file_body, Fields>& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base().base() = rep.base();
			this->body().file() = rep.body();
		}

		http_response_impl_t(http::message<false, http::file_body, Fields>&& rep)
			: super()
		#ifdef ASIO2_ENABLE_HTTP_RESPONSE_USER_DATA
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		#endif
		{
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
		}

		self& operator=(const http::message<false, http::file_body, Fields>& rep)
		{
			this->base().base() = rep.base();
			this->body().file() = rep.body();
			return *this;
		}

		self& operator=(http::message<false, http::file_body, Fields>&& rep)
		{
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
			return *this;
		}

		/**
		 * @brief destructor
		 */
		~http_response_impl_t()
		{
		}

		/// Returns the base portion of the message
		inline super const& base() const noexcept
		{
			return *this;
		}

		/// Returns the base portion of the message
		inline super& base() noexcept
		{
			return *this;
		}

		inline void reset()
		{
			static_cast<super&>(*this) = {};

			this->result(http::status::unknown);
		}

		/**
		 * @brief set the root directory where we load the files.
		 */
		inline self& set_root_directory(const std::filesystem::path& path)
		{
			std::error_code ec{};
			this->root_directory_ = std::filesystem::canonical(path, ec);
			assert(!ec);
			return *this;
		}

		/**
		 * @brief get the root directory where we load the files.
		 */
		inline const std::filesystem::path& get_root_directory() noexcept
		{
			return this->root_directory_;
		}

		/**
		 * @brief create a deferred http response, the response will not be send immediately,
		 * the http response will be sent only when the returned std::shared_ptr<http::response_defer>
		 * is completely destroyed
		 */
		inline std::shared_ptr<http::response_defer> defer()
		{
			this->defer_guard_ = std::make_shared<http::response_defer>(
				this->defer_callback_, this->session_ptr_.lock());
			return this->defer_guard_;
		}

	public:
		/**
		 * @brief Respond to http request with plain text content
		 * @param content - the response body, it's usually a simple string,
		 * and the content-type is "text/plain" by default.
		 */
		template<class StringT>
		inline self& fill_text(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "text/plain", unsigned version = 11)
		{
			// must clear file_body
			this->body().file().close();

			this->set(http::field::server, BEAST_VERSION_STRING);
			this->set(http::field::content_type, mimetype.empty() ? "text/plain" : mimetype);

			this->result(result);
			this->version(version < 10 ? 11 : version);

			this->body().text() = detail::to_string(std::forward<StringT>(content));
			http::try_prepare_payload(*this);

			return (*this);
		}

		/**
		 * @brief Respond to http request with json content
		 */
		template<class StringT>
		inline self& fill_json(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "application/json", unsigned version = 11)
		{
			return this->fill_text(std::forward<StringT>(content), result,
				mimetype.empty() ? "application/json" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with html content
		 * @param content - the response body, may be a plain text string, or a stardand
		 * <html>...</html> string, it's just that the content-type is "text/html" by default.
		 */
		template<class StringT>
		inline self& fill_html(StringT&& content, http::status result = http::status::ok,
			std::string_view mimetype = "text/html", unsigned version = 11)
		{
			return this->fill_text(std::forward<StringT>(content), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with pre-prepared error page content
		 * Generated a standard html error page automatically use the status coe 'result',
		 * like <html>...</html>, and the content-type is "text/html" by default.
		 */
		template<class StringT = std::string_view>
		inline self& fill_page(http::status result, StringT&& desc = std::string_view{},
			std::string_view mimetype = "text/html", unsigned version = 11)
		{
			return this->fill_text(http::error_page(result, std::forward<StringT>(desc)), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @brief Respond to http request with local file
		 */
		inline self& fill_file(std::filesystem::path path,
			http::status result = http::status::ok, unsigned version = 11)
		{
			// if you want to build a absolute path by youself and passed it to fill_file function,
			// call set_root_directory("") first, then passed you absolute path to fill_file is ok.

			// Build the path to the requested file
			std::filesystem::path filepath;

			if (this->root_directory_.empty())
			{
				filepath = std::move(path);
			}
			else
			{
				filepath = detail::make_filepath(this->root_directory_, path);
			}

			filepath.make_preferred();

			// Attempt to open the file
			beast::error_code ec;
			this->body().file().open(filepath.string().c_str(), beast::file_mode::scan, ec);

			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
				return this->fill_page(http::status::not_found, {}, {}, version);

			// Handle an unknown error
			if (ec)
				return this->fill_page(http::status::not_found, ec.message(), {}, version);

			// Cache the size since we need it after the move
			auto const size = this->body().size();

			// Respond to GET request
			this->content_length(size);

			this->set(http::field::server, BEAST_VERSION_STRING);
			this->set(http::field::content_type, http::extension_to_mimetype(path.extension().string()));

			this->result(result);
			this->version(version < 10 ? 11 : version);

			return (*this);
		}

		/**
		 * @brief Returns `true` if this HTTP response's Content-Type is "multipart/form-data";
		 */
		inline bool has_multipart() const noexcept
		{
			return http::has_multipart(*this);
		}

		/**
		 * @brief Get the "multipart/form-data" body content.
		 */
		inline decltype(auto) get_multipart()
		{
			return http::multipart(*this);
		}

		/**
		 * @brief Get the "multipart/form-data" body content. same as get_multipart
		 */
		inline decltype(auto) multipart()
		{
			return this->get_multipart();
		}

	protected:
		std::filesystem::path                    root_directory_     = std::filesystem::current_path();

		std::function<void()>                    defer_callback_;

		std::shared_ptr<http::response_defer>    defer_guard_;

		std::weak_ptr<void>                      session_ptr_;
	};
}

#ifdef ASIO2_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	using web_response = asio2::detail::http_response_impl_t<http::flex_body>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_RESPONSE_IMPL_HPP__
