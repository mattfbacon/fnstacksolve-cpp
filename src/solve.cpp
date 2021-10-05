#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

#include "solve.hpp"

bool check_solution(
	math_value_t const input_value,
	math_value_t const desired_output,
	std::span<MathExpression::MathExpression> const expressions,
	std::vector<SolutionItem>& possible_solution,
	Environment& environment) {
	auto current_value = input_value;
	auto const num_expressions = possible_solution.size();
	for (size_t i = 0; i < num_expressions; i++) {
		auto const expression_index = possible_solution[i].idx;
		environment.set_independent(current_value);
		current_value = possible_solution[i].step_result_value = expressions[expression_index].evaluate(environment);
	}
	// fuzzy equals
	return fabs(current_value - desired_output) < std::numeric_limits<math_value_t>::epsilon();
}

std::optional<Solution> solve(math_value_t const input_value, math_value_t const desired_output, std::span<MathExpression::MathExpression> const expressions) {
	auto environment = Environment::default_environment(input_value);

	Solution solution(expressions.size());
	for (size_t i = 0; i < expressions.size(); i++) {
		solution[i].idx = i;
	}

#if 1
	do {
		if (check_solution(input_value, desired_output, expressions, solution, environment)) {
			return { solution };
		}
	} while (std::next_permutation(solution.begin(), solution.end(), [](auto const& a, auto const& b) {
		return b.idx > a.idx;
	}));
#else
	(void)desired_output;
	for (auto const& expression : expressions) {
		std::cout << expression.get_original_text() << " => " << expression.evaluate(environment) << '\n';
	}
#endif
	return std::nullopt;
}
