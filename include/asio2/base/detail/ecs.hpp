/*
 * Copyright (c) 2017-2023 zhllxt
 *
 * author   : zhllxt
 * email    : 37792738@qq.com
 * 
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef __ASIO2_ECS_HPP__
#define __ASIO2_ECS_HPP__

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <tuple>

#include <asio2/base/detail/match_condition.hpp>

#include <asio2/component/rdc/rdc_option.hpp>
#include <asio2/component/socks/socks5_option.hpp>

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
					return detail::to_shared_ptr(ecs_t{ std::move(args)... });
				else
					return detail::to_shared_ptr(ecs_t{ component_t{std::move(args)...} });
			}
			else
			{
				if constexpr (sizeof...(Args) == std::size_t(0))
					return detail::to_shared_ptr(ecs_t{ std::move(c), std::move(args)... });
				else
					return detail::to_shared_ptr(ecs_t{ component_t{std::move(c), std::move(args)...} });
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

#endif // !__ASIO2_ECS_HPP__
