/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 * refrenced from : mqtt_cpp/include/mqtt/broker/security.hpp
 */

#ifndef __ASIO2_MQTT_SECURITY_HPP__
#define __ASIO2_MQTT_SECURITY_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <algorithm>
#include <variant>
#include <map>
#include <set>
#include <optional>

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/iterator/function_output_iterator.hpp>)
#include <boost/iterator/function_output_iterator.hpp>
#else
#include <asio2/bho/iterator/function_output_iterator.hpp>
#endif

#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/algorithm/hex.hpp>)
#include <boost/algorithm/hex.hpp>
#else
#include <asio2/bho/algorithm/hex.hpp>
#endif

#include <asio2/base/iopool.hpp>
#include <asio2/util/string.hpp>

#if !defined(INCLUDE_NLOHMANN_JSON_HPP_) && !defined(NLOHMANN_JSON_HPP)
#include <asio2/external/json.hpp>
#endif

#include <asio2/mqtt/message.hpp>

#include <asio2/mqtt/detail/mqtt_topic_util.hpp>
#include <asio2/mqtt/detail/mqtt_subscription_map.hpp>

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
#include <openssl/evp.h>
#endif

namespace asio2::mqtt
{

struct security
{
	static constexpr char const* any_group_name = "@any";

	struct authentication
	{
		enum class method : std::uint8_t
		{
			sha256,
			plain_password,
			client_cert,
			anonymous,
			unauthenticated
		};

		authentication(
			method auth_method = method::sha256,
			std::optional<std::string> digest = std::nullopt,
			std::string salt = std::string()
		)
			: auth_method(auth_method)
			, digest(std::move(digest))
			, salt(std::move(salt))
		{
		}

		method auth_method;
		std::optional<std::string> digest;
		std::string salt;

		std::vector<std::string> groups;
	};

	struct authorization
	{
		enum class type : std::uint8_t
		{
			deny, allow, none
		};

		authorization(std::string_view topic, std::size_t rule_nr)
			: topic(topic)
			, rule_nr(rule_nr)
			, sub_type(type::none)
			, pub_type(type::none)
		{
		}

		std::vector<std::string> topic_tokens;

		std::string topic;
		std::size_t rule_nr;

		type sub_type;
		std::set<std::string> sub;

		type pub_type;
		std::set<std::string> pub;
	};

	struct group
	{
		std::string name;
		std::vector<std::string> members;
	};

	/** Return username of anonymous user */
	std::optional<std::string> const& login_anonymous() const
	{
		return this->anonymous_;
	}

	/** Return username of unauthorized user */
	std::optional<std::string> const& login_unauthenticated() const
	{
		return this->unauthenticated_;
	}

	template<typename T>
	static std::string to_hex(T start, T end)
	{
		std::string result;
	#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/algorithm/hex.hpp>)
		boost::algorithm::hex(start, end, std::back_inserter(result));
	#else
		bho::algorithm::hex(start, end, std::back_inserter(result));
	#endif
		return result;
	}

#if defined(ASIO2_ENABLE_SSL) || defined(ASIO2_USE_SSL)
    static std::string sha256hash(string_view message) {
        std::shared_ptr<EVP_MD_CTX> mdctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
        EVP_DigestInit_ex(mdctx.get(), EVP_sha256(), NULL);
        EVP_DigestUpdate(mdctx.get(), message.data(), message.size());

        std::vector<unsigned char> digest(static_cast<std::size_t>(EVP_MD_size(EVP_sha256())));
        unsigned int digest_size = static_cast<unsigned int>(digest.size());

        EVP_DigestFinal_ex(mdctx.get(), digest.data(), &digest_size);
        return to_hex(digest.data(), digest.data() + digest_size);
    }
#else
	static std::string sha256hash(std::string_view message)
	{
		return std::string(message);
	}
#endif

	bool login_cert(std::string_view username) const
	{
		auto i = authentication_.find(std::string(username));
		return
			i != authentication_.end() &&
			i->second.auth_method == security::authentication::method::client_cert;
	}

	std::optional<std::string> login(std::string_view username, std::string_view password) const
	{
		auto i = authentication_.find(std::string(username));
		if (i != authentication_.end() &&
			i->second.auth_method == security::authentication::method::sha256)
		{
			return [&]() -> std::optional<std::string>
			{
				if (asio2::iequals(
					i->second.digest.value(),
					sha256hash(i->second.salt + std::string(password))))
				{
					return std::string(username);
				}
				else
				{
					return std::nullopt;
				}
			} ();
		}
		else if (
			i != authentication_.end() &&
			i->second.auth_method == security::authentication::method::plain_password)
		{
			return [&]() -> std::optional<std::string>
			{
				if (i->second.digest.value() == password)
				{
					return std::string(username);
				}
				else
				{
					return std::nullopt;
				}
			} ();
		}
		return std::nullopt;
	}

    static authorization::type get_auth_type(std::string_view type)
    {
        if (type == "allow") return authorization::type::allow;
        if (type == "deny") return authorization::type::deny;
        throw std::runtime_error(
            "An invalid authorization type was specified: " +
            std::string(type)
        );
    }

    static bool is_valid_group_name(std::string_view name)
    {
        return !name.empty() && name[0] == '@'; // TODO: validate utf-8
    }

    static bool is_valid_user_name(std::string_view name)
    {
        return !name.empty() && name[0] != '@'; // TODO: validate utf-8
    }

	std::size_t get_next_rule_nr() const
	{
		std::size_t rule_nr = 0;
		for (auto const& i : authorization_)
		{
			rule_nr = (std::max)(rule_nr, i.rule_nr);
		}
		return rule_nr + 1;
	}

	void default_config()
	{
		char const* username = "anonymous";
		authentication login(authentication::method::anonymous);
		authentication_.emplace(username, login);
		anonymous_ = username;

		char const* topic = "#";
		authorization auth(topic, get_next_rule_nr());
		auth.topic_tokens = get_topic_filter_tokens("#");
		auth.sub_type = authorization::type::allow;
		auth.sub.emplace(username);
		auth.pub_type = authorization::type::allow;
		auth.pub.emplace(username);
		authorization_.push_back(auth);

		groups_.emplace(std::string(any_group_name), group());

		validate();
	}

    std::size_t add_auth(
        std::string const& topic_filter,
        std::set<std::string> const& pub,
        authorization::type auth_pub_type,
        std::set<std::string> const& sub,
        authorization::type auth_sub_type
    )
    {
        for(auto const& j : pub)
        {
            if (!is_valid_user_name(j) && !is_valid_group_name(j))
            {
                throw std::runtime_error(
                    "An invalid username or groupname was specified for the authorization: " + j
                );
            }
            validate_entry("topic " + topic_filter, j);
        }

        for(auto const& j : sub)
        {
            if (!is_valid_user_name(j) && !is_valid_group_name(j))
            {
                throw std::runtime_error(
                    "An invalid username or groupname was specified for the authorization: " + j
                );
            }
            validate_entry("topic " + topic_filter, j);
        }

        std::size_t rule_nr = get_next_rule_nr();
        authorization auth(topic_filter, rule_nr);
        auth.topic_tokens = get_topic_filter_tokens(topic_filter);
        auth.pub = pub;
        auth.pub_type = auth_pub_type;
        auth.sub = sub;
        auth.sub_type = auth_sub_type;

        for (auto const& j: sub) 
        {
            auth_sub_map_.insert_or_assign(
                topic_filter,
                j,
                std::make_pair(auth_sub_type, rule_nr)
            );
        }
        for (auto const& j: pub)
        {
            auth_pub_map_.insert_or_assign(
                topic_filter,
                j,
                std::make_pair(auth_pub_type, rule_nr)
            );
        }

        authorization_.push_back(auth);
        return rule_nr;
    }

	void remove_auth(std::size_t rule_nr)
	{
		for (auto i = authorization_.begin(); i != authorization_.end(); ++i)
		{
			if (i->rule_nr == rule_nr)
			{
				for (auto const& j : i->sub)
				{
					auth_sub_map_.erase(i->topic, j);
				}
				for (auto const& j : i->pub)
				{
					auth_pub_map_.erase(i->topic, j);
				}

				authorization_.erase(i);
				return;
			}
		}
	}

	void add_sha256_authentication(std::string username, std::string digest, std::string salt)
	{
		authentication auth(authentication::method::sha256, std::move(digest), std::move(salt));
		authentication_.emplace(std::move(username), std::move(auth));
	}

	void add_plain_password_authentication(std::string username, std::string password)
	{
		authentication auth(authentication::method::plain_password, std::move(password));
		authentication_.emplace(std::move(username), std::move(auth));
	}

	void add_certificate_authentication(std::string username)
	{
		authentication auth(authentication::method::client_cert);
		authentication_.emplace(std::move(username), std::move(auth));
	}

	void load_json(std::istream& input)
	{
		using json = nlohmann::json;

		json j;
		input >> j;

		groups_.emplace(std::string(any_group_name), group());

		if (auto& j_authentication = j["authentication"]; j_authentication.is_array())
		{
			for (auto& i : j_authentication)
			{
				auto& j_name = i["name"];
				if (!j_name.is_string())
				{
					ASIO2_ASSERT(false);
					continue;
				}

				std::string name = j_name.get<std::string>();
				if (!is_valid_user_name(name))
				{
					ASIO2_ASSERT(false);
					continue;
				}

				auto& j_method = i["method"];
				if (!j_method.is_string())
				{
					ASIO2_ASSERT(false);
					continue;
				}

				std::string method = j_method.get<std::string>();

				if (method == "sha256")
				{
					auto& j_digest = i["digest"];
					auto& j_salt = i["salt"];

					if (j_digest.is_string())
					{
						std::string digest = j_digest.get<std::string>();
						std::string salt;

						if (j_salt.is_string())
							salt = j_salt.get<std::string>();

						authentication auth(authentication::method::sha256, std::move(digest), std::move(salt));
						authentication_.emplace(std::move(name), std::move(auth));
					}
				}
				else if (method == "plain_password")
				{
					auto& j_password = i["password"];

					if (j_password.is_string())
					{
						std::string digest = j_password.get<std::string>();

						authentication auth(authentication::method::plain_password, digest);
						authentication_.emplace(std::move(name), std::move(auth));
					}
				}
				else if (method == "client_cert")
				{
					authentication auth(authentication::method::client_cert);
					authentication_.emplace(std::move(name), std::move(auth));
				}
				else if (method == "anonymous")
				{
					if (anonymous_)
					{
						ASIO2_ASSERT(false && "Only a single anonymous user can be configured");
					}
					else
					{
						anonymous_ = name;

						authentication auth(authentication::method::anonymous);
						authentication_.emplace(std::move(name), std::move(auth));
					}
				}
				else if (method == "unauthenticated")
				{
					if (unauthenticated_)
					{
						ASIO2_ASSERT(false && "Only a single unauthenticated user can be configured");
					}
					else
					{
						unauthenticated_ = name;

						authentication auth(authentication::method::unauthenticated);
						authentication_.emplace(std::move(name), std::move(auth));
					}
				}
				else
				{
					ASIO2_ASSERT(false);
				}
			}
		}

		if (auto& j_groups = j["groups"]; j_groups.is_array())
		{
			for (auto& i : j_groups)
			{
				auto& j_name = i["name"];
				if (!j_name.is_string())
				{
					ASIO2_ASSERT(false);
					continue;
				}

				std::string name = j_name.get<std::string>();
				if (!is_valid_group_name(name))
				{
					ASIO2_ASSERT(false);
					continue;
				}

				group group;

				if (auto& j_members = j["members"]; j_members.is_array())
				{
					for (auto& j_username : j_members)
					{
						if (!j_username.is_string())
						{
							ASIO2_ASSERT(false);
							continue;
						}

						auto username = j_username.get<std::string>();

						if (!is_valid_user_name(username))
						{
							ASIO2_ASSERT(false);
							continue;
						}

						group.members.emplace_back(std::move(username));
					}
				}
				else
				{
					ASIO2_ASSERT(false);
				}

				groups_.emplace(std::move(name), std::move(group));
			}
		}

		if (auto& j_authorization = j["authorization"]; j_authorization.is_array())
		{
			for (auto& i : j_authorization)
			{
				auto& j_name = i["topic"];
				if (!j_name.is_string())
				{
					ASIO2_ASSERT(false);
					continue;
				}

				std::string name = j_name.get<std::string>();
				if (!validate_topic_filter(name))
				{
					ASIO2_ASSERT(false);
					continue;
				}

				authorization auth(name, get_next_rule_nr());
				auth.topic_tokens = get_topic_filter_tokens(name);

				if (auto& j_allow = i["allow"]; j_allow.is_object())
				{
					if (auto& j_sub = j_allow["sub"]; j_sub.is_array())
					{
						for (auto& j_username : j_sub)
						{
							if (j_username.is_string())
							{
								auth.sub.emplace(j_username.get<std::string>());
							}
						}
						auth.sub_type = authorization::type::allow;
					}

					if (auto& j_pub = j_allow["pub"]; j_pub.is_array())
					{
						for (auto& j_username : j_pub)
						{
							if (j_username.is_string())
							{
								auth.pub.emplace(j_username.get<std::string>());
							}
						}
						auth.pub_type = authorization::type::allow;
					}
				}

				if (auto& j_deny = i["deny"]; j_deny.is_object())
				{
					if (auto& j_sub = j_deny["sub"]; j_sub.is_array())
					{
						for (auto& j_username : j_sub)
						{
							if (j_username.is_string())
							{
								auth.sub.emplace(j_username.get<std::string>());
							}
						}
						auth.sub_type = authorization::type::deny;
					}

					if (auto& j_pub = j_deny["pub"]; j_pub.is_array())
					{
						for (auto& j_username : j_pub)
						{
							auth.pub.emplace(j_username.get<std::string>());
						}
						auth.pub_type = authorization::type::deny;
					}

				}
				authorization_.emplace_back(std::move(auth));
			}
		}

		validate();
	}

	template<typename T>
	void get_auth_sub_by_user(std::string_view username, T&& callback) const
	{
		std::set<std::string> username_and_groups;
		username_and_groups.insert(std::string(username));

		for (auto const& i : groups_)
		{
			if (i.first == any_group_name ||
				std::find(i.second.members.begin(), i.second.members.end(), username) != i.second.members.end())
			{
				username_and_groups.insert(i.first);
			}
		}

		for (auto const& i : authorization_)
		{
			if (i.sub_type != authorization::type::none)
			{
				bool sets_intersect = false;
				auto store_intersect = [&sets_intersect](std::string const&) mutable
				{
					sets_intersect = true;
				};

				std::set_intersection(
					i.sub.begin(),
					i.sub.end(),
					username_and_groups.begin(),
					username_and_groups.end(),
				#if !defined(ASIO2_HEADER_ONLY) && __has_include(<boost/iterator/function_output_iterator.hpp>)
					boost::make_function_output_iterator(std::ref(store_intersect))
				#else
					bho::make_function_output_iterator(std::ref(store_intersect))
				#endif
				);

				if (sets_intersect)
				{
					std::forward<T>(callback)(i);
				}
			}
		}
	}

	authorization::type auth_pub(std::string_view topic, std::string_view username)
	{
		authorization::type result_type = authorization::type::deny;

		std::set<std::string> username_and_groups;
		username_and_groups.insert(std::string(username));

		for (auto const& i : groups_)
		{
			if (i.first == any_group_name ||
				std::find(i.second.members.begin(), i.second.members.end(), username) != i.second.members.end())
			{
				username_and_groups.insert(i.first);
			}
		}

		std::size_t priority = 0;
		auth_pub_map_.match(topic,
			[&](const std::string& allowed_username, std::pair<authorization::type, std::size_t>& entry) mutable
			{
				if (username_and_groups.find(allowed_username) != username_and_groups.end())
				{
					if (entry.second >= priority)
					{
						result_type = entry.first;
						priority = entry.second;
					}
				}
			}
		);

		return result_type;
	}

	std::map<std::string, authorization::type> auth_sub(std::string_view topic)
	{
		std::map<std::string, authorization::type> result;

		std::size_t priority = 0;
		auth_sub_map_.match(topic,
			[&](const std::string& allowed_username, std::pair<authorization::type, std::size_t>& entry)
			{
				if (entry.second >= priority)
				{
					result[allowed_username] = entry.first;
					priority = entry.second;
				}
			}
		);

		return result;
	}

	authorization::type auth_sub_user(
		std::map<std::string, authorization::type> const& result, std::string const& username)
	{
		auto it = result.find(username);
		if (it != result.end())
			return it->second;

		for (auto& [k, v] : groups_)
		{
			if (k == any_group_name ||
				std::find(v.members.begin(), v.members.end(), username) != v.members.end())
			{
				auto j = result.find(k);
				if (j != result.end())
					return j->second;
			}
		}

		return authorization::type::deny;
	}

	static bool is_hash(std::string_view level) { return level == "#"; }
	static bool is_plus(std::string_view level) { return level == "+"; }
	static bool is_literal(std::string_view level) { return !is_hash(level) && !is_plus(level); }

	static std::optional<std::string> is_subscribe_allowed(
		std::vector<std::string> const& authorized_filter,std::string_view subscription_filter)
	{
		std::optional<std::string> result;
		auto append_result = [&result](std::string_view token)
		{
			if (result)
			{
				result.value() += topic_filter_separator;
				result.value().append(token.data(), token.size());
			}
			else
			{
				result = std::string(token);
			}
		};

		auto filter_begin = authorized_filter.begin();

		auto subscription_begin = subscription_filter.begin();
		auto subscription_next = topic_filter_tokenizer_next(subscription_begin, subscription_filter.end());

		while (true)
		{
			if (filter_begin == authorized_filter.end())
			{
				return std::nullopt;
			}

			auto auth = *filter_begin;
			++filter_begin;

			if (is_hash(auth))
			{
				append_result(std::string_view(&(*subscription_begin),
					std::distance(subscription_begin, subscription_filter.end())));
				return result;
			}

			auto sub = std::string_view(&(*subscription_begin),
				std::distance(subscription_begin, subscription_next));

			if (is_hash(sub))
			{
				append_result(auth);

				while (filter_begin < authorized_filter.end())
				{
					append_result(*filter_begin);
					++filter_begin;
				}

				return result;
			}

			if (is_plus(auth))
			{
				append_result(sub);
			}
			else if (is_plus(sub))
			{
				append_result(auth);
			}
			else
			{
				if (auth != sub)
				{
					return std::nullopt;
				}

				append_result(auth);
			}

			if (subscription_next == subscription_filter.end())
				break;

			subscription_begin = std::next(subscription_next);
			subscription_next = topic_filter_tokenizer_next(subscription_begin, subscription_filter.end());
		}

		if (filter_begin < authorized_filter.end())
		{
			return std::nullopt;
		}

		return result;
	}

	static bool is_subscribe_denied(
		std::vector<std::string> const& deny_filter, std::string_view subscription_filter)
	{
		bool result = true;
		auto filter_begin = deny_filter.begin();

		auto tokens_count = topic_filter_tokenizer(subscription_filter,
			[&](auto sub)
			{
				if (filter_begin == deny_filter.end())
				{
					result = false;
					return false;
				};

				std::string deny = *filter_begin;
				++filter_begin;

				if (deny != sub)
				{
					if (is_hash(deny))
					{
						result = true;
						return false;
					}

					if (is_hash(sub))
					{
						result = false;
						return false;
					}

					if (is_plus(deny))
					{
						result = true;
						return true;
					}

					result = false;
					return false;
				}

				return true;
			}
		);

		return result && (tokens_count == deny_filter.size());
	}

	std::vector<std::string> get_auth_sub_topics(std::string_view username, std::string_view topic_filter) const
	{
		std::vector<std::string> auth_topics;
		get_auth_sub_by_user(username,
			[&](authorization const& i)
			{
				if (i.sub_type == authorization::type::allow)
				{
					auto entry = is_subscribe_allowed(i.topic_tokens, topic_filter);
					if (entry)
					{
						auth_topics.push_back(entry.value());
					}
				}
				else
				{
					for (auto j = auth_topics.begin(); j != auth_topics.end();)
					{
						if (is_subscribe_denied(i.topic_tokens, topic_filter))
						{
							j = auth_topics.erase(j);
						}
						else
						{
							++j;
						}
					}
				}
			}
		);
		return auth_topics;
	}

	/**
	 * @brief Determine if user is allowed to subscribe to the specified topic filter
	 * @param username - The username to check
	 * @param topic_filter - Topic filter the user would like to subscribe to
	 * @return true if the user is authorized
	 */
	bool is_subscribe_authorized(std::string_view username, std::string_view topic_filter) const
	{
		return !get_auth_sub_topics(username, topic_filter).empty();
	}

	// Get the individual path elements of the topic filter
	static std::vector<std::string> get_topic_filter_tokens(std::string_view topic_filter)
	{
		std::vector<std::string> result;
		topic_filter_tokenizer(topic_filter,
			[&result](auto str)
			{
				result.push_back(std::string(str));
				return true;
			}
		);

		return result;
	}

	inline bool enabled() const noexcept { return enabled_; }
	inline void enabled(bool v) noexcept { enabled_ = v; }

	bool enabled_ = true;

	std::map<std::string, authentication> authentication_;
	std::map<std::string, group> groups_;

	std::vector<authorization> authorization_;

	std::optional<std::string> anonymous_;
	std::optional<std::string> unauthenticated_;

	using auth_map_type = subscription_map<std::string, std::pair<authorization::type, std::size_t>>;

	auth_map_type auth_pub_map_;
	auth_map_type auth_sub_map_;

protected:
	void validate_entry(std::string const& context, std::string const& name) const
	{
		if (is_valid_group_name(name) && groups_.find(name) == groups_.end())
		{
			throw std::runtime_error("An invalid group name was specified for " + context + ": " + name);
		}
		if (is_valid_user_name(name) && authentication_.find(name) == authentication_.end())
		{
			throw std::runtime_error("An invalid username name was specified for " + context + ": " + name);
		}
	}

	void validate()
	{
		for (auto const& i : groups_)
		{
			for (auto const& j : i.second.members)
			{
				auto iter = authentication_.find(j);
				if (is_valid_user_name(j) && iter == authentication_.end())
					throw std::runtime_error("An invalid username name was specified for group " + i.first + ": " + j);
			}
		}

		std::string unsalted;
		for (auto const& i : authentication_)
		{
			if (i.second.auth_method == authentication::method::sha256 && i.second.salt.empty())
			{
				if (!unsalted.empty()) unsalted += ", ";
				unsalted += i.first;
			}
		}

		if (!unsalted.empty())
		{
			//MQTT_LOG("mqtt_broker", warning)
			//    << "The following users have no salt specified: "
			//    << unsalted;
		}

		for (auto const& i : authorization_)
		{
			for (auto const& j : i.sub)
			{
				validate_entry("topic " + i.topic, j);

				if (is_valid_user_name(j) || is_valid_group_name(j))
				{
					auth_sub_map_.insert_or_assign(i.topic, j, std::make_pair(i.sub_type, i.rule_nr));
				}
			}
			for (auto const& j : i.pub)
			{
				validate_entry("topic " + i.topic, j);

				if (is_valid_user_name(j) || is_valid_group_name(j))
				{
					auth_pub_map_.insert_or_assign(i.topic, j, std::make_pair(i.pub_type, i.rule_nr));
				}
			}
		}
	}
};

}

#endif // __ASIO2_MQTT_SECURITY_HPP__
