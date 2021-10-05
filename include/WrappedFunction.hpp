#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

#include "common.hpp"

class WrappedFunction {
public:
	using value_t = ::math_value_t;
	struct ArityError : public std::exception {
		size_t given_args;
		size_t needed_args;
		ArityError(auto given_args, auto needed_args) : given_args(given_args), needed_args(needed_args) {}
		char const* what() const noexcept override {
			return "ArityError";
		}
	};
protected:
	template <size_t K, typename... Ts>
	struct KaryFunctionTypeExpander {
		using type_t = typename KaryFunctionTypeExpander<K - 1, value_t, Ts...>::type_t;
	};
	template <typename... Ts>
	struct KaryFunctionTypeExpander<0, Ts...> {
		using type_t = value_t (*)(Ts...);
	};
	template <size_t K>
	using function_type_t = typename KaryFunctionTypeExpander<K>::type_t;
protected:
	template <size_t K, size_t... Idxs>
	static value_t function_call_helper(function_type_t<K> fn, std::vector<value_t> const& args, std::index_sequence<Idxs...>) {
		return fn(args[Idxs]...);
	}
protected:
	std::function<value_t(std::vector<value_t> const&)> function;
	size_t arity;
public:
	WrappedFunction() {
		arity = std::numeric_limits<size_t>::max();
	}
	template <size_t Arity>
	static WrappedFunction create(function_type_t<Arity> actual_function) {
		auto function = [actual_function](std::vector<value_t> const& args) {
			if (size_t const args_size = args.size(); args_size != Arity) {
				throw ArityError{ args_size, Arity };
			}
			return function_call_helper<Arity>(actual_function, args, std::make_index_sequence<Arity>{});
		};
		return WrappedFunction{ std::move(function), Arity };
	}
	WrappedFunction(decltype(function) function, size_t arity) : function(std::move(function)), arity(std::move(arity)) {}

	bool can_call(std::vector<value_t> const& args) const {
		return arity != std::numeric_limits<size_t>::max() && args.size() == arity;
	}
	size_t get_arity() const {
		return arity;
	}
	auto call(auto const& args) const {
		assert(arity != std::numeric_limits<size_t>::max());
		return function(args);
	}
};
