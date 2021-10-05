#include <cassert>
#include <charconv>
#include <cstring>
#include <iostream>

#include "tao/pegtl.hpp"
#include "tao/pegtl/contrib/parse_tree.hpp"
#ifndef NDEBUG
#include "tao/pegtl/contrib/analyze.hpp"
#include "tao/pegtl/contrib/parse_tree_to_dot.hpp"
#include "tao/pegtl/contrib/trace.hpp"
#endif

#include "MathExpression.hpp"

namespace MathExpression {

namespace P = tao::pegtl;

namespace Parser {

// This grammar is based on the Parse Tree example from the PEGTL repo.

struct integer : P::plus<P::digit> {};
/*
 * If number starts with a dot, then match an integer and interpret the whole thing as a decimal.
 * Otherwise, match an integer. If the integer is followed by a dot, then match the dot and another integer directly after it.
 */
struct number_base : P::if_then_else<P::one<'.'>, integer, P::seq<integer, P::opt<P::seq<P::one<'.'>, integer>>>> {};
struct number : P::seq<P::opt<P::one<'-'>>, number_base> {};
struct variable : P::identifier {};
struct function_name : P::identifier {};

struct plus : P::pad<P::one<'+'>, P::space> {};
struct minus : P::pad<P::one<'-'>, P::space> {};
struct multiply : P::pad<P::one<'*'>, P::space> {};
struct divide : P::pad<P::one<'/'>, P::space> {};
struct exponent : P::pad<P::one<'^'>, P::space> {};

struct open_bracket : P::seq<P::one<'('>, P::star<P::space>> {};
struct close_bracket : P::seq<P::star<P::space>, P::one<')'>> {};

struct prec_layer_0;
struct bracketed : P::seq<open_bracket, prec_layer_0, close_bracket> {};
struct function_application : P::seq<function_name, open_bracket, P::list<prec_layer_0, P::one<','>, P::space>, close_bracket> {};

template <typename Child, typename... Operators>
struct left_assoc_operator : P::list<Child, P::sor<Operators...>, P::space> {};
template <typename Child, typename... Operators>
struct right_assoc_operator : P::seq<P::pad<Child, P::space>, P::opt<P::seq<P::sor<Operators...>, right_assoc_operator<Child, Operators...>>>> {};

struct prec_layer_3 : P::sor<number, function_application, variable, bracketed> {};
struct prec_layer_2 : right_assoc_operator<prec_layer_3, exponent> {};
struct prec_layer_1 : left_assoc_operator<prec_layer_2, multiply, divide> {};
struct prec_layer_0 : left_assoc_operator<prec_layer_1, plus, minus> {};

struct grammar : P::must<P::pad<prec_layer_0, P::space>, P::eof> {};

template <bool RightAssoc = false>
struct operator_rearrange : P::parse_tree::apply<operator_rearrange<RightAssoc>> {
	template <typename... States>
	static void transform(std::unique_ptr<P::parse_tree::node>& node, States&&... st) {
		if (node->children.size() == 1) {
			node = std::move(node->children[0]);
		} else {
			assert(node->children.size() > 1 && node->children.size() % 2 == 1);
			node->remove_content();
			auto& children = node->children;
			auto& first_operand = node;
			auto second_operand = std::move(children.back());
			children.pop_back();
			auto operator_ = std::move(children.back());
			children.pop_back();
			operator_->children.emplace_back(std::move(first_operand));
			operator_->children.emplace_back(std::move(second_operand));
			node = std::move(operator_);
			auto& recurse_child = node->children.front();
			if (recurse_child->is_type<prec_layer_0>() || recurse_child->is_type<prec_layer_1>() || recurse_child->is_type<prec_layer_2>()) {
				transform(recurse_child, st...);
			}
		}
	}
};
template <>
struct operator_rearrange<true> : P::parse_tree::apply<operator_rearrange<true>> {
	template <typename... States>
	static void transform(std::unique_ptr<P::parse_tree::node>& node, States&&... st) {
		if (node->children.size() == 1) {
			node = std::move(node->children[0]);
		} else {
			assert(node->children.size() > 1 && node->children.size() % 2 == 1);
			node->remove_content();
			auto& children = node->children;
			auto first_operand = std::move(children[0]);
			auto& second_operand = node;
			auto operator_ = std::move(children[1]);
			operator_->children.emplace_back(std::move(first_operand));
			// take the rest of the children, starting at the third (index 2), since the first two have already been "stolen"
			second_operand->children = std::vector(std::make_move_iterator(children.begin() + 2), std::make_move_iterator(children.end()));
			operator_->children.emplace_back(std::move(second_operand));
			node = std::move(operator_);
			auto& recurse_child = node->children.back();
			if (recurse_child->is_type<prec_layer_0>() || recurse_child->is_type<prec_layer_1>() || recurse_child->is_type<prec_layer_2>()) {
				transform(recurse_child, st...);
			}
		}
	}
};

struct function_as_node : P::parse_tree::apply<function_as_node> {
	template <typename... States>
	static void transform(std::unique_ptr<P::parse_tree::node>& node, States&&...) {
		// transforms from...
		//      function_application
		//  function_name  arg  arg  arg
		// ...to...
		//    function_name
		//  arg    arg    arg

		// first remove any content from the node
		node->remove_content();
		auto& children = node->children;
		assert(children.size() >= 1);
		// take the name and store it for later
		auto function_name_node = std::move(children.front());
		children.erase(children.begin());  // children is now just the arguments
		using std::swap;
		// make the arguments the children of the function name
		swap(function_name_node->children, node->children);
		// and finally make the node be the function name
		node = std::move(function_name_node);
	}
};

// clang-format off
template <typename Rule>
using selector = P::parse_tree::selector<Rule
	, P::parse_tree::store_content::on<number, variable, function_name>
	, P::parse_tree::remove_content::on<plus, minus, multiply, divide, exponent>
	, function_as_node::on<function_application>
	, operator_rearrange<false>::on<prec_layer_0, prec_layer_1>
	, operator_rearrange<true>::on<prec_layer_2>
>;
// clang-format on

}  // namespace Parser

MathExpression::MathExpression(std::string_view const to_parse) : original_text{ to_parse } {
	tao::pegtl::memory_input<tao::pegtl::tracking_mode::eager, tao::pegtl::ascii::eol::lf_crlf, std::string> input{ original_text, std::string{} };
#ifndef NDEBUG
	auto const issues = P::analyze<Parser::grammar>();
	assert(issues == 0);
	P::standard_trace<Parser::grammar, Parser::selector>(input);
	input.restart();
#endif
	parse_root = P::parse_tree::parse<Parser::grammar, Parser::selector>(input);
	assert(parse_root != nullptr);
#ifndef NDEBUG
	P::parse_tree::print_dot(std::cout, *parse_root);
#endif
}

value_t MathExpression::evaluate(Environment const& environment) const {
	assert(parse_root->is_root());
	assert(parse_root->children.size() == 1);
	return evaluate_node(parse_root->children[0], environment);
}

value_t MathExpression::evaluate_node(node_t const& node, Environment const& environment) {
	auto const& node_children = node->children;
	if (node->is_type<Parser::number>()) {
		assert(node->children.empty());
		assert(node->has_content());
		value_t ret = 0;
		auto const [ptr, ec] = std::from_chars(node->string_view().begin(), node->string_view().end(), ret);
		assert(ptr == node->string_view().end());
		assert(ec != std::errc::invalid_argument);
		if (ec == std::errc::result_out_of_range) {
			throw PrecisionError{ node->string_view(), node->begin(), node->end() };
		}
		return ret;
	} else if (node->is_type<Parser::variable>()) {
		assert(node->children.empty());
		assert(node->has_content());
		auto const value = environment.get_variable(node->string());
		if (!value.has_value()) {
			throw NameError{ node->string_view(), NameError::Type::variable, node->begin(), node->end() };
		}
		return *value;
	} else if (node->is_type<Parser::plus>()) {
		assert(node_children.size() == 2 && node_children[0] != nullptr && node_children[1] != nullptr);
		return evaluate_node(node_children[0], environment) + evaluate_node(node_children[1], environment);
	} else if (node->is_type<Parser::minus>()) {
		assert(node_children.size() == 2 && node_children[0] != nullptr && node_children[1] != nullptr);
		return evaluate_node(node_children[0], environment) - evaluate_node(node_children[1], environment);
	} else if (node->is_type<Parser::multiply>()) {
		assert(node_children.size() == 2 && node_children[0] != nullptr && node_children[1] != nullptr);
		return evaluate_node(node_children[0], environment) * evaluate_node(node_children[1], environment);
	} else if (node->is_type<Parser::divide>()) {
		assert(node_children.size() == 2 && node_children[0] != nullptr && node_children[1] != nullptr);
		return evaluate_node(node_children[0], environment) / evaluate_node(node_children[1], environment);
	} else if (node->is_type<Parser::exponent>()) {
		assert(node_children.size() == 2 && node_children[0] != nullptr && node_children[1] != nullptr);
		return pow(evaluate_node(node_children[0], environment), evaluate_node(node_children[1], environment));
	} else if (node->is_type<Parser::function_name>()) {
		assert(node->has_content());
		auto const function_name = node->string_view();
		auto const function = environment.get_function(node->string());
		if (!function.has_value()) {
			throw NameError{ function_name, NameError::Type::function, node->begin(), node->end() };
		}
		auto const& node_children = node->children;
		std::vector<value_t> argument_values(node_children.size(), 0);  // fill constructor
		std::transform(node_children.cbegin(), node_children.cend(), argument_values.begin(), [&environment](auto const& child_node) {
			return evaluate_node(child_node, environment);
		});
		return function->get().call(argument_values);
	} else {
		throw std::runtime_error{ std::string{ "Unexpected node type showed up in " } + __PRETTY_FUNCTION__ };
	}
}

void parse_tree_move_helper(MathExpression::node_t& root, char const* old_loc, char const* new_loc) {
	if (root.get() == nullptr) return;
	if (root->source.data() != nullptr && !root->source.empty()) root->source = new_loc;
	// get the offset from the beginning of the old string, and add that to the new string
	root->m_begin.data = new_loc + (root->m_begin.data - old_loc);
	root->m_end.data = new_loc + (root->m_end.data - old_loc);
	for (auto& child : root->children) {
		parse_tree_move_helper(child, old_loc, new_loc);
	}
}

MathExpression::MathExpression(MathExpression&& other) noexcept {
	char const* const old_loc = other.original_text.data();
	original_text = std::move(other.original_text);
	parse_root = std::move(other.parse_root);
	// make sure that the parse_root points to the new location of original_text
	if (char const* const new_loc = original_text.data(); old_loc != new_loc) {
		parse_tree_move_helper(parse_root, old_loc, original_text.data());
	}
}

}  // namespace MathExpression
