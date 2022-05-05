/*
 * COPYRIGHT (C) 2017-2021, zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
 * (See accompanying file LICENSE or see <http://www.gnu.org/licenses/>)
 */

#ifndef __ASIO2_HTTP_RESPONSE_IMPL_HPP__
#define __ASIO2_HTTP_RESPONSE_IMPL_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <asio2/base/detail/push_options.hpp>

#include <filesystem>

#include <asio2/base/component/user_data_cp.hpp>

#include <asio2/http/detail/flex_body.hpp>
#include <asio2/http/detail/http_util.hpp>

#ifdef BEAST_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	class response_defer
	{
	public:
		response_defer(std::function<void()> cb, std::shared_ptr<void> session)
			: cb_(std::move(cb)), session_(std::move(session))
		{
			ASIO2_ASSERT(session_);
		}
		~response_defer()
		{
			if (cb_) { cb_(); }
		}

	protected:
		std::function<void()> cb_;

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
#ifndef ASIO2_DISABLE_HTTP_RESPONSE_USER_DATA_CP
		, public user_data_cp<http_response_impl_t<Body, Fields>>
#endif
	{
		ASIO2_CLASS_FRIEND_DECLARE_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_BASE;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_CLIENT;
		ASIO2_CLASS_FRIEND_DECLARE_TCP_SESSION;

	public:
		using self  = http_response_impl_t<Body, Fields>;
		using super = http::message<false, Body, Fields>;
		using header_type = typename super::header_type;
		using body_type = typename super::body_type;

	public:
		/**
		 * @constructor
		 */
		template<typename... Args>
		explicit http_response_impl_t(Args&&... args)
			: super(std::forward<Args>(args)...)
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
		}

		http_response_impl_t(const http_response_impl_t& o)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base() = o.base();
			this->root_directory_ = o.root_directory_;
			this->defer_callback_ = o.defer_callback_;
			this->defer_guard_    = o.defer_guard_;
			this->session_ptr_    = o.session_ptr_;
		}

		http_response_impl_t(http_response_impl_t&& o)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
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
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base() = rep;
		}

		template<class BodyT = Body>
		http_response_impl_t(http::message<false, BodyT, Fields>&& rep)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
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
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base().base() = rep.base();
			this->body().text() = rep.body();
			this->prepare_payload();
		}

		http_response_impl_t(http::message<false, http::string_body, Fields>&& rep)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
			this->prepare_payload();
		}

		self& operator=(const http::message<false, http::string_body, Fields>& rep)
		{
			this->base().base() = rep.base();
			this->body().text() = rep.body();
			this->prepare_payload();
			return *this;
		}

		self& operator=(http::message<false, http::string_body, Fields>&& rep)
		{
			this->base().base() = std::move(rep.base());
			this->body().text() = std::move(rep.body());
			this->prepare_payload();
			return *this;
		}

		//-------------------------------------------------

		http_response_impl_t(const http::message<false, http::file_body, Fields>& rep)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base().base() = rep.base();
			this->body().file() = rep.body();
			this->prepare_payload();
		}

		http_response_impl_t(http::message<false, http::file_body, Fields>&& rep)
			: super()
			, user_data_cp<http_response_impl_t<Body, Fields>>()
		{
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
			this->prepare_payload();
		}

		self& operator=(const http::message<false, http::file_body, Fields>& rep)
		{
			this->base().base() = rep.base();
			this->body().file() = rep.body();
			this->prepare_payload();
			return *this;
		}

		self& operator=(http::message<false, http::file_body, Fields>&& rep)
		{
			this->base().base() = std::move(rep.base());
			this->body().file() = std::move(rep.body());
			this->prepare_payload();
			return *this;
		}

		/**
		 * @destructor
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
		 * @function : set the root directory where we load the files.
		 */
		inline self& set_root_directory(std::filesystem::path path)
		{
			this->root_directory_ = std::move(path);
			return *this;
		}

		/**
		 * @function : set the root directory where we load the files. same as set_root_directory
		 */
		inline self& root_directory(std::filesystem::path path)
		{
			return this->set_root_directory(std::move(path));
		}

		/**
		 * @function : get the root directory where we load the files.
		 */
		inline const std::filesystem::path& get_root_directory() noexcept
		{
			return this->root_directory_;
		}

		/**
		 * @function : get the root directory where we load the files. same as get_root_directory
		 */
		inline const std::filesystem::path& root_directory() noexcept
		{
			return this->get_root_directory();
		}

		/**
		 * @function : create a deferred http response, the response will not be send immediately,
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
		 * @function : Respond to http request with plain text content
		 * @param : content - the response body, it's usually a simple string,
		 * and the content-type is "text/plain" by default.
		 */
		inline self& fill_text(std::string content, http::status result = http::status::ok,
			std::string_view mimetype = "text/plain", unsigned version = 11)
		{
			// must clear file_body
			this->body().file().close();

			this->set(http::field::server, BEAST_VERSION_STRING);
			this->set(http::field::content_type, mimetype.empty() ? "text/plain" : mimetype);

			this->result(result);
			this->version(version < 10 ? 11 : version);

			this->body().text() = std::move(content);
			this->prepare_payload();

			return (*this);
		}

		/**
		 * @function : Respond to http request with json content
		 */
		inline self& fill_json(std::string content, http::status result = http::status::ok,
			std::string_view mimetype = "application/json", unsigned version = 11)
		{
			return this->fill_text(std::move(content), result,
				mimetype.empty() ? "application/json" : mimetype, version);
		}

		/**
		 * @function : Respond to http request with html content
		 * @param : content - the response body, may be a plain text string, or a stardand
		 * <html>...</html> string, it's just that the content-type is "text/html" by default.
		 */
		inline self& fill_html(std::string content, http::status result = http::status::ok,
			std::string_view mimetype = "text/html", unsigned version = 11)
		{
			return this->fill_text(std::move(content), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @function : Respond to http request with pre-prepared error page content
		 * Generated a standard html error page automatically use the status coe 'result',
		 * like <html>...</html>, and the content-type is "text/html" by default.
		 */
		inline self& fill_page(http::status result, std::string desc = {},
			std::string_view mimetype = "text/html", unsigned version = 11)
		{
			return this->fill_text(http::error_page(result, std::move(desc)), result,
				mimetype.empty() ? "text/html" : mimetype, version);
		}

		/**
		 * @function : Respond to http request with local file
		 */
		inline self& fill_file(std::filesystem::path path,
			http::status result = http::status::ok, unsigned version = 11)
		{
			path.make_preferred();

			std::filesystem::path filepath;

			if (path.is_absolute())
			{
				filepath = std::move(path);
			}
			else
			{
				// Build the path to the requested file
				filepath = this->root_directory_;
				filepath.make_preferred();
				filepath /= path.relative_path();
			}

			// Attempt to open the file
			beast::error_code ec;
			this->body().file().open(filepath.string().c_str(), beast::file_mode::scan, ec);

			// Handle the case where the file doesn't exist
			if (ec == beast::errc::no_such_file_or_directory)
				return this->fill_page(http::status::not_found, {}, {}, version);

			// Handle an unknown error
			if (ec)
				return this->fill_page(http::status::internal_server_error, ec.message(), {}, version);

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
		 * @function : Returns `true` if this HTTP response's Content-Type is "multipart/form-data";
		 */
		inline bool has_multipart() noexcept
		{
			return http::has_multipart(*this);
		}

		/**
		 * @function : Get the "multipart/form-data" body content.
		 */
		inline decltype(auto) get_multipart()
		{
			return http::multipart(*this);
		}

		/**
		 * @function : Get the "multipart/form-data" body content. same as get_multipart
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

#ifdef BEAST_HEADER_ONLY
namespace bho::beast::http
#else
namespace boost::beast::http
#endif
{
	using web_response = asio2::detail::http_response_impl_t<http::flex_body>;
}

#include <asio2/base/detail/pop_options.hpp>

#endif // !__ASIO2_HTTP_RESPONSE_IMPL_HPP__
