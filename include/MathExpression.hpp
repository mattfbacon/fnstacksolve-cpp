#pragma once

#include <cmath>
#include <stdexcept>
#include <string>

#include "tao/pegtl/contrib/parse_tree.hpp"
#include "tao/pegtl/memory_input.hpp"
#include "tao/pegtl/position.hpp"

#include "Environment.hpp"

#include "common.hpp"

namespace MathExpression {

using value_t = ::math_value_t;

class MathExpression {
public:
	using node_t = std::unique_ptr<tao::pegtl::parse_tree::node>;
	struct PrecisionError : public std::exception {
		std::string content;
		tao::pegtl::position start_position, end_position;
		PrecisionError(auto content, auto start_position, auto end_position)
			: content(std::move(content)), start_position(std::move(start_position)), end_position(std::move(end_position)) {}
		char const* what() const noexcept override {
			return "PrecisionError";
		}
	};
	struct NameError : public std::exception {
		enum class Type {
			function,
			variable,
		};
		std::string content;
		Type type;
		tao::pegtl::position start_position, end_position;
		NameError(auto content, auto type, auto start_position, auto end_position)
			: content(std::move(content)), type(std::move(type)), start_position(std::move(start_position)), end_position(std::move(end_position)) {}
		char const* what() const noexcept override {
			return "NameError";
		}
	};
public:
	explicit MathExpression(std::string_view to_parse);
	MathExpression(MathExpression const&) = delete;
	MathExpression(MathExpression&&) noexcept;
	static value_t evaluate_node(node_t const& node, Environment const& environment);
	value_t evaluate(Environment const& environment) const;
	std::string_view get_original_text() const {
		return original_text;
	}
protected:
	node_t parse_root;
	std::string original_text;
};

// operators

#ifdef HARD_CODED_FUNCTION_WRAPPERS
class BinaryFunction : public Base {
	using fnptr_t = value_t (*)(value_t, value_t);
	fnptr_t fn;
	std::unique_ptr<Base> left, right;
public:
	BinaryFunction(fnptr_t fn, std::unique_ptr<Base>&& left, std::unique_ptr<Base>&& right) : fn(fn), left(std::move(left)), right(std::move(right)) {}
	value_t evaluate(value_t const independent) const override {
		return fn(left->evaluate(independent), right->evaluate(independent));
	}
};
#endif

#ifdef WEIRD_FUNCTION_EXPANDERS
template <size_t K, typename... Ts>
struct KaryFunctionTypeExpander {
	using recurse_t = KaryFunctionTypeExpander<K - 1, Base::value_t, Ts...>;
	using type_t = Base::value_t (*)(Ts...);
};

template <typename... Ts>
struct KaryFunctionTypeExpander<0, Ts...> {
	using recurse_t = KaryFunctionTypeExpander<0, Ts...>;
};
#endif

#ifdef FINAL_REVISION
template <size_t K>
class KaryFunction : public Base {
	using fnptr_t = value_t (*)(std::array<value_t, K> const&);
	fnptr_t fn;
	std::array<std::unique_ptr<Base>, K> args;
public:
	template <typename... Ts>
	KaryFunction(fnptr_t fn, Ts&&... args) : fn(fn), args{ std::move(args)... } {}
	value_t evaluate(value_t const independent) const override {
		std::array<value_t, K> evaluated;
		std::transform(args.cbegin(), args.cend(), evaluated.begin(), [independent](std::unique_ptr<Base> const& x) {
			return x->evaluate(independent);
		});
		return fn(evaluated);
	}
};
#endif

}  // namespace MathExpression
