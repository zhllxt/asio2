/*
 * COPYRIGHT (C) 2017-2021, zhllxt
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
#include <tuple>
#include <type_traits>
#include <functional>

#include <asio2/ecs/rdc/rdc_option.hpp>
#include <asio2/ecs/socks/socks5_option.hpp>

namespace asio2::detail
{
	namespace
	{
		using iterator = asio::buffers_iterator<asio::streambuf::const_buffers_type>;
		using diff_type = typename iterator::difference_type;
		std::pair<iterator, bool> dgram_match_role(iterator begin, iterator end) noexcept
		{
			for (iterator p = begin; p < end;)
			{
				// If 0~253, current byte are the payload length.
				if (std::uint8_t(*p) < std::uint8_t(254))
				{
					std::uint8_t payload_size = static_cast<std::uint8_t>(*p);

					++p;

					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
				}

				// If 254, the following 2 bytes interpreted as a 16-bit unsigned integer
				// are the payload length.
				if (std::uint8_t(*p) == std::uint8_t(254))
				{
					++p;

					if (end - p < 2)
						break;

					std::uint16_t payload_size = *(reinterpret_cast<const std::uint16_t*>(p.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::uint16_t)>(reinterpret_cast<std::uint8_t*>(
							std::addressof(payload_size)));
					}

					// illegal data
					if (payload_size < static_cast<std::uint16_t>(254))
						return std::pair(begin, true);

					p += 2;
					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
				}

				// If 255, the following 8 bytes interpreted as a 64-bit unsigned integer
				// (the most significant bit MUST be 0) are the payload length.
				if (std::uint8_t(*p) == 255)
				{
					++p;

					if (end - p < 8)
						break;

					// the most significant bit MUST be 0
					std::int64_t payload_size = *(reinterpret_cast<const std::int64_t*>(p.operator->()));

					// use little endian
					if (!is_little_endian())
					{
						swap_bytes<sizeof(std::int64_t)>(reinterpret_cast<std::uint8_t*>(
							std::addressof(payload_size)));
					}

					// illegal data
					if (payload_size <= static_cast<std::int64_t>((std::numeric_limits<std::uint16_t>::max)()))
						return std::pair(begin, true);

					p += 8;
					if (end - p < static_cast<diff_type>(payload_size))
						break;

					return std::pair(p + static_cast<diff_type>(payload_size), true);
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

		condition_traits(condition_traits&&) noexcept = default;
		condition_traits(condition_traits const&) = default;
		condition_traits& operator=(condition_traits&&) noexcept = default;
		condition_traits& operator=(condition_traits const&) = default;

		// must use explicit, Otherwise, there will be an error when there are the following
		// statements: condition_traits<char> c1; auto c2 = c1;
		template<class MT, std::enable_if_t<
			!std::is_base_of_v<condition_traits, detail::remove_cvref_t<MT>>, int> = 0>
		explicit condition_traits(MT c) noexcept : condition_(std::move(c)) {}

		inline T& operator()() noexcept { return this->condition_; }

	protected:
		T condition_;
	};

	// C++17 class template argument deduction guides
	template<class T>
	condition_traits(T)->condition_traits<std::remove_reference_t<T>>;

	template<>
	class condition_traits<void>
	{
	public:
		using type = void;

		condition_traits(condition_traits&&) noexcept = default;
		condition_traits(condition_traits const&) noexcept = default;
		condition_traits& operator=(condition_traits&&) noexcept = default;
		condition_traits& operator=(condition_traits const&) noexcept = default;

		 condition_traits() noexcept = default;
		~condition_traits() noexcept = default;

		inline void operator()() noexcept {}
	};

	template<>
	class condition_traits<use_dgram_t>
	{
	public:
		using type = use_dgram_t;

		condition_traits(condition_traits&&) noexcept = default;
		condition_traits(condition_traits const&) noexcept = default;
		condition_traits& operator=(condition_traits&&) noexcept = default;
		condition_traits& operator=(condition_traits const&) noexcept = default;

		explicit condition_traits(use_dgram_t) noexcept {}
		~condition_traits() noexcept = default;

		inline auto& operator()() noexcept { return dgram_match_role; }

	protected:
	};

	template<>
	class condition_traits<use_kcp_t>
	{
	public:
		using type = use_kcp_t;

		condition_traits(condition_traits&&) noexcept = default;
		condition_traits(condition_traits const&) noexcept = default;
		condition_traits& operator=(condition_traits&&) noexcept = default;
		condition_traits& operator=(condition_traits const&) noexcept = default;

		explicit condition_traits(use_kcp_t) noexcept {}
		~condition_traits() noexcept = default;

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }

	protected:
	};

	template<>
	class condition_traits<hook_buffer_t>
	{
	public:
		using type = hook_buffer_t;

		condition_traits(condition_traits&&) noexcept = default;
		condition_traits(condition_traits const&) noexcept = default;
		condition_traits& operator=(condition_traits&&) noexcept = default;
		condition_traits& operator=(condition_traits const&) noexcept = default;

		explicit condition_traits(hook_buffer_t) noexcept {}
		~condition_traits() noexcept = default;

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }

	protected:
	};
}

namespace asio2::detail
{
	template<class ConditionT, class... Args>
	struct ecs_t
	{
		static constexpr std::size_t componentc = sizeof...(Args);

		using type       = ConditionT;
		using tuple_type = std::tuple<typename detail::shared_ptr_adapter<Args>::type...>;

		template<std::size_t I>
		struct components
		{
			static_assert(I < componentc, "index is out of range, index must less than sizeof Args");
			using type = typename std::tuple_element<I, tuple_type>::type;
		};

		ecs_t(ecs_t&&) = default;
		ecs_t(ecs_t const&) = delete;
		ecs_t& operator=(ecs_t&&) = default;
		ecs_t& operator=(ecs_t const&) = delete;

		template<std::size_t... I>
		inline ecs_t clone_impl(std::index_sequence<I...>) noexcept
		{
			return ecs_t{ this->condition_, (*std::get<I>(this->components_))... };
		}

		inline ecs_t clone()
		{
			return clone_impl(std::make_index_sequence<componentc>{});
		}

		template<class Condition, class... Ts>
		explicit ecs_t(Condition&& c, Ts&&... ts)
			: condition_(std::forward<Condition>(c))
			, components_(detail::to_shared_ptr(std::forward<Ts>(ts))...)
		{
		}

		inline decltype(auto) operator()() noexcept { return condition_(); }

		template<typename = void>
		static constexpr bool has_rdc() noexcept
		{
			return (is_template_instance_of_v<asio2::rdc::option,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type> || ...);
		}

		template<std::size_t I, typename T1, typename... TN>
		static constexpr std::size_t rdc_index_helper() noexcept
		{
			if constexpr (is_template_instance_of_v<asio2::rdc::option, T1>)
				return I;
			else
			{
				if constexpr (sizeof...(TN) == 0)
					return std::size_t(0);
				else
					return rdc_index_helper<I + 1, TN...>();
			}
		}

		template<typename = void>
		static constexpr std::size_t rdc_index() noexcept
		{
			return rdc_index_helper<0,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type...>();
		}

		template<class Tag, std::enable_if_t<std::is_same_v<Tag, std::in_place_t>, int> = 0>
		typename components<rdc_index()>::type rdc_option(Tag) noexcept
		{
			return std::get<rdc_index()>(components_);
		}

		template<typename = void>
		static constexpr bool has_socks5() noexcept
		{
			return (std::is_base_of_v<asio2::socks5::detail::option_base,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type> || ...);
		}

		template<std::size_t I, typename T1, typename... TN>
		static constexpr std::size_t socks5_index_helper() noexcept
		{
			if constexpr (std::is_base_of_v<asio2::socks5::detail::option_base, T1>)
				return I;
			else
			{
				if constexpr (sizeof...(TN) == 0)
					return std::size_t(0);
				else
					return socks5_index_helper<I + 1, TN...>();
			}
		}

		template<typename = void>
		static constexpr std::size_t socks5_index() noexcept
		{
			return socks5_index_helper<0,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type...>();
		}

		template<class Tag, std::enable_if_t<std::is_same_v<Tag, std::in_place_t>, int> = 0>
		typename components<socks5_index()>::type socks5_option(Tag) noexcept
		{
			return std::get<socks5_index()>(components_);
		}

		condition_traits<type>           condition_;
		tuple_type                       components_;
	};

	// C++17 class template argument deduction guides
	template<class C, class... Ts>
	ecs_t(C, Ts...)->ecs_t<C, Ts...>;
}

namespace asio2::detail
{
	template<class T>
	class condition_wrap
	{
	public:
		using traits_type = condition_traits<T>;
		using condition_type = typename traits_type::type;

		 condition_wrap() noexcept = default;
		~condition_wrap() noexcept = default;

		condition_wrap(condition_wrap&&) noexcept = default;
		condition_wrap(condition_wrap const&) = default;
		condition_wrap& operator=(condition_wrap&&) noexcept = default;
		condition_wrap& operator=(condition_wrap const&) = default;

		inline condition_wrap clone()
		{
			return condition_wrap{ this->impl_ };
		}

		template<class MT, std::enable_if_t<
			!std::is_base_of_v<condition_wrap, detail::remove_cvref_t<MT>>, int> = 0>
		explicit condition_wrap(MT c) noexcept : impl_(std::move(c)) {}

		explicit condition_wrap(traits_type c) noexcept : impl_(std::move(c)) {}

		inline decltype(auto) operator()() noexcept { return impl_(); }

	protected:
		traits_type impl_;
	};

	// C++17 class template argument deduction guides
	template<class T>
	condition_wrap(T)->condition_wrap<std::remove_reference_t<T>>;

	template<>
	class condition_wrap<void>
	{
	public:
		using type = void;
		using condition_type = void;

		 condition_wrap() noexcept = default;
		~condition_wrap() noexcept = default;

		condition_wrap(condition_wrap&&) noexcept = default;
		condition_wrap(condition_wrap const&) noexcept = default;
		condition_wrap& operator=(condition_wrap&&) noexcept = default;
		condition_wrap& operator=(condition_wrap const&) noexcept = default;

		inline condition_wrap clone()
		{
			return condition_wrap{};
		}
	};

	template<class ConditionT, class... Args>
	class condition_wrap<ecs_t<ConditionT, Args...>>
	{
	public:
		using traits_type = ecs_t<ConditionT, Args...>;
		using condition_type = typename traits_type::type;

		 condition_wrap() noexcept = default;
		~condition_wrap() noexcept = default;

		condition_wrap(condition_wrap&&) noexcept = default;
		condition_wrap(condition_wrap const&) = default;
		condition_wrap& operator=(condition_wrap&&) noexcept = default;
		condition_wrap& operator=(condition_wrap const&) = default;

		inline condition_wrap clone()
		{
			return condition_wrap{ this->impl_->clone() };
		}

		template<class ECST, std::enable_if_t<
			!std::is_base_of_v<condition_wrap, detail::remove_cvref_t<ECST>>, int> = 0>
		explicit condition_wrap(ECST c)
			: impl_(std::make_shared<detail::remove_cvref_t<ECST>>(std::move(c))) {}

		inline decltype(auto) operator()() noexcept { return (*impl_)(); }

		std::shared_ptr<traits_type> impl_;
	};
}

namespace asio2::detail
{
	struct condition_helper
	{
		template<class T>
		static constexpr bool is_component() noexcept
		{
			using type = detail::remove_cvref_t<T>;

			if constexpr (is_template_instance_of_v<std::shared_ptr, type>)
			{
				return is_component<typename type::element_type>();
			}
			else
			{
				if /**/ constexpr (is_template_instance_of_v<asio2::rdc::option, type>)
					return true;
				else if constexpr (std::is_base_of_v<asio2::socks5::detail::option_base, type>)
					return true;
				else
					return false;
			}
		}

		template<class... Args>
		static constexpr bool has_match_condition() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return false;
			else
				return (!(condition_helper::is_component<Args>() && ...));
		}

		// use "DefaultConditionT c, Args... args", not use "DefaultConditionT&& c, Args&&... args"
		// to avoid the "c and args..." variables being references
		template<class DefaultConditionT, class... Args>
		static constexpr auto make_condition(DefaultConditionT c, Args... args) noexcept
		{
			detail::ignore_unused(c);

			if constexpr (condition_helper::has_match_condition<Args...>())
			{
				if constexpr (sizeof...(Args) == std::size_t(1))
					return condition_wrap{ std::move(args)... };
				else
					return condition_wrap{ ecs_t{std::move(args)...} };
			}
			else
			{
				if constexpr (sizeof...(Args) == std::size_t(0))
					return condition_wrap{ std::move(c), std::move(args)... };
				else
					return condition_wrap{ ecs_t{std::move(c), std::move(args)...} };
			}
		}

		template<typename MatchCondition>
		static constexpr bool has_rdc() noexcept
		{
			if constexpr (is_template_instance_of_v<ecs_t, MatchCondition>)
			{
				return MatchCondition::has_rdc();
			}
			else
			{
				return false;
			}
		}

		template<typename MatchCondition>
		static constexpr bool has_socks5() noexcept
		{
			if constexpr (is_template_instance_of_v<ecs_t, MatchCondition>)
			{
				return MatchCondition::has_socks5();
			}
			else
			{
				return false;
			}
		}
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
