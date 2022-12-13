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
	/**
	 * use condition_t to avoid having the same name as make_error_condition(condition c)
	 */
	template<class T>
	class condition_t
	{
	public:
		using type = T;

		// must use explicit, Otherwise, there will be an error when there are the following
		// statements: condition_t<char> c1; auto c2 = c1;
		template<class C, std::enable_if_t<!std::is_base_of_v<condition_t, detail::remove_cvref_t<C>>, int> = 0>
		explicit condition_t(C c) noexcept : c_(std::move(c)) {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ this->c_ }; }

		inline T& operator()() noexcept { return this->c_; }
		inline T& lowest    () noexcept { return this->c_; }

	protected:
		T c_;
	};

	// C++17 class template argument deduction guides
	template<class T>
	condition_t(T)->condition_t<std::remove_reference_t<T>>;

	template<>
	class condition_t<void>
	{
	public:
		using type = void;

		 condition_t() noexcept = default;
		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{}; }

		inline void operator()() noexcept {}
		inline void lowest    () noexcept {}
	};

	template<>
	class condition_t<use_dgram_t>
	{
	public:
		using type = use_dgram_t;

		explicit condition_t(use_dgram_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ use_dgram_t{} }; }

		inline auto& operator()() noexcept { return dgram_match_role; }
		inline auto& lowest    () noexcept { return dgram_match_role; }
	};

	template<>
	class condition_t<use_kcp_t>
	{
	public:
		using type = use_kcp_t;

		explicit condition_t(use_kcp_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ use_kcp_t{} }; }

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }
		inline asio::detail::transfer_at_least_t lowest    () noexcept { return asio::transfer_at_least(1); }
	};

	template<>
	class condition_t<hook_buffer_t>
	{
	public:
		using type = hook_buffer_t;

		explicit condition_t(hook_buffer_t) noexcept {}

		~condition_t() noexcept = default;

		condition_t(condition_t&&) noexcept = default;
		condition_t(condition_t const&) noexcept = delete;
		condition_t& operator=(condition_t&&) noexcept = default;
		condition_t& operator=(condition_t const&) noexcept = delete;

		inline condition_t clone() noexcept { return condition_t{ hook_buffer_t{} }; }

		inline asio::detail::transfer_at_least_t operator()() noexcept { return asio::transfer_at_least(1); }
		inline asio::detail::transfer_at_least_t lowest    () noexcept { return asio::transfer_at_least(1); }
	};
}

namespace asio2::detail
{
	struct component_tag {};

	struct component_dummy {};

	template<class ConditionT, class... Args>
	struct component_t : public component_tag
	{
		static constexpr std::size_t componentc = sizeof...(Args);

		using components_type = std::tuple<typename detail::shared_ptr_adapter<Args>::type...>;

		using condition_type = condition_t<typename detail::remove_cvref_t<ConditionT>>;
		using condition_lowest_type = typename condition_type::type;

		template<std::size_t I>
		struct components
		{
			static_assert(I < componentc, "index is out of range, index must less than sizeof Args");
			using type     = typename std::tuple_element<I, components_type>::type;
			using raw_type = typename element_type_adapter<type>::type;
		};

		component_t(component_t&&) noexcept = default;
		component_t(component_t const&) = delete;
		component_t& operator=(component_t&&) noexcept = default;
		component_t& operator=(component_t const&) = delete;

		template<std::size_t... I>
		inline component_t clone_impl(std::index_sequence<I...>) noexcept
		{
			return component_t{ this->condition_.clone(),
				(typename components<I>::raw_type(*std::get<I>(this->components_)))... };
		}

		inline component_t clone()
		{
			return clone_impl(std::make_index_sequence<componentc>{});
		}

		template<class LowestT, class... Ts>
		explicit component_t(LowestT&& c, Ts&&... ts)
			: condition_(std::forward<LowestT>(c))
			, components_(detail::to_shared_ptr(std::forward<Ts>(ts))...)
		{
		}

		inline decltype(auto) operator()() noexcept { return condition_(); }

		template<typename = void>
		static constexpr bool has_rdc() noexcept
		{
			return (std::is_base_of_v<asio2::rdc::option_base,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type> || ...);
		}

		template<std::size_t I, typename T1, typename... TN>
		static constexpr std::size_t rdc_index_helper() noexcept
		{
			if constexpr (std::is_base_of_v<asio2::rdc::option_base, T1>)
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
		asio2::rdc::option_base* rdc_option_addr() noexcept
		{
			if constexpr (has_rdc())
			{
				return this->rdc_option(std::in_place).get();
			}
			else
			{
				return nullptr;
			}
		}

		template<typename = void>
		static constexpr bool has_socks5() noexcept
		{
			return (std::is_base_of_v<asio2::socks5::option_base,
				typename element_type_adapter<typename detail::remove_cvref_t<Args>>::type> || ...);
		}

		template<std::size_t I, typename T1, typename... TN>
		static constexpr std::size_t socks5_index_helper() noexcept
		{
			if constexpr (std::is_base_of_v<asio2::socks5::option_base, T1>)
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

		inline condition_type        & get_condition() noexcept { return condition_; }
		inline components_type       & values       () noexcept { return components_; }

	protected:
		condition_type                 condition_;
		components_type                components_;
	};

	// C++17 class template argument deduction guides
	template<class C, class... Ts>
	component_t(C, Ts...)->component_t<C, Ts...>;
}

namespace asio2::detail
{
	/**
	 * 
	 */
	class ecs_base
	{
	public:
		virtual ~ecs_base() {}
		virtual asio2::rdc::option_base* get_rdc() noexcept { return nullptr; }
	};

	template<class T>
	class ecs_t : public ecs_base
	{
	public:
		using component_type = component_dummy;
		using condition_type = condition_t<T>;
		using condition_lowest_type = typename condition_type::type;

		 ecs_t() noexcept = default;
		~ecs_t() noexcept = default;

		ecs_t(ecs_t&&) noexcept = default;
		ecs_t(ecs_t const&) = delete;
		ecs_t& operator=(ecs_t&&) noexcept = default;
		ecs_t& operator=(ecs_t const&) = delete;

		inline ecs_t clone() noexcept { return ecs_t{ this->condition_.clone() }; }

		template<class LowestT, std::enable_if_t<!std::is_base_of_v<ecs_t, detail::remove_cvref_t<LowestT>>, int> = 0>
		explicit ecs_t(LowestT c) noexcept : condition_(std::move(c)) {}

		explicit ecs_t(condition_type c) noexcept : condition_(std::move(c)) {}

		inline condition_type& get_condition() noexcept { return condition_; }
		inline component_type& get_component() noexcept { return component_; }

	protected:
		condition_type condition_;
		component_type component_;
	};

	// C++17 class template argument deduction guides
	template<class T>
	ecs_t(T)->ecs_t<std::remove_reference_t<T>>;

	template<>
	class ecs_t<void> : public ecs_base
	{
	public:
		using component_type = component_dummy;
		using condition_type = condition_t<void>;
		using condition_lowest_type = void;

		 ecs_t() noexcept = default;
		~ecs_t() noexcept = default;

		ecs_t(ecs_t&&) noexcept = default;
		ecs_t(ecs_t const&) noexcept = delete;
		ecs_t& operator=(ecs_t&&) noexcept = default;
		ecs_t& operator=(ecs_t const&) noexcept = delete;

		inline ecs_t clone() noexcept { return ecs_t{}; }

		inline condition_type& get_condition() noexcept { return condition_; }
		inline component_type& get_component() noexcept { return component_; }

	protected:
		condition_type condition_;
		component_type component_;
	};

	template<class ConditionT, class... Args>
	class ecs_t<component_t<ConditionT, Args...>> : public ecs_base
	{
	public:
		using component_type = component_t<ConditionT, Args...>;
		using condition_type = typename component_type::condition_type;
		using condition_lowest_type = typename condition_type::type;

		 ecs_t() noexcept = default;
		~ecs_t() noexcept = default;

		ecs_t(ecs_t&&) noexcept = default;
		ecs_t(ecs_t const&) = delete;
		ecs_t& operator=(ecs_t&&) noexcept = default;
		ecs_t& operator=(ecs_t const&) = delete;

		inline ecs_t clone() noexcept { return ecs_t{ this->component_.clone() }; }

		template<class C, std::enable_if_t<!std::is_base_of_v<ecs_t, detail::remove_cvref_t<C>>, int> = 0>
		explicit ecs_t(C c) : component_(std::move(c)) {}

		inline condition_type& get_condition() noexcept { return component_.get_condition(); }
		inline component_type& get_component() noexcept { return component_; }

		virtual asio2::rdc::option_base* get_rdc() noexcept override
		{
			return this->component_.rdc_option_addr();
		}

	protected:
		component_type component_;
	};
}

namespace asio2::detail
{
	struct ecs_helper
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
				if /**/ constexpr (std::is_base_of_v<asio2::rdc::option_base, type>)
					return true;
				else if constexpr (std::is_base_of_v<asio2::socks5::option_base, type>)
					return true;
				else
					return false;
			}
		}

		template<class... Args>
		static constexpr bool args_has_match_condition() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return false;
			else
				return (!(ecs_helper::is_component<Args>() && ...));
		}

		template<class... Args>
		static constexpr bool args_has_rdc() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return false;
			else
				return (std::is_base_of_v<asio2::rdc::option_base, detail::remove_cvref_t<Args>> || ...);
		}

		template<class... Args>
		static constexpr bool args_has_socks5() noexcept
		{
			if constexpr (sizeof...(Args) == std::size_t(0))
				return false;
			else
				return (std::is_base_of_v<asio2::socks5::option_base, detail::remove_cvref_t<Args>> || ...);
		}

		// use "DefaultConditionT c, Args... args", not use "DefaultConditionT&& c, Args&&... args"
		// to avoid the "c and args..." variables being references
		template<class DefaultConditionT, class... Args>
		static constexpr auto make_ecs(DefaultConditionT c, Args... args) noexcept
		{
			detail::ignore_unused(c);

			if constexpr (ecs_helper::args_has_match_condition<Args...>())
			{
				if constexpr (sizeof...(Args) == std::size_t(1))
					return ecs_t{ std::move(args)... };
				else
					return ecs_t{ component_t{std::move(args)...} };
			}
			else
			{
				if constexpr (sizeof...(Args) == std::size_t(0))
					return ecs_t{ std::move(c), std::move(args)... };
				else
					return ecs_t{ component_t{std::move(c), std::move(args)...} };
			}
		}

		template<typename C>
		static constexpr bool has_rdc() noexcept
		{
			if constexpr (std::is_base_of_v<component_tag, detail::remove_cvref_t<C>>)
			{
				return C::has_rdc();
			}
			else
			{
				return false;
			}
		}

		template<typename C>
		static constexpr bool has_socks5() noexcept
		{
			if constexpr (std::is_base_of_v<component_tag, detail::remove_cvref_t<C>>)
			{
				return C::has_socks5();
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
