#pragma once

#include <cstddef>
#include <optional>
#include <span>

#include "MathExpression.hpp"

#include "common.hpp"

struct SolutionItem {
	size_t idx = 0;
	math_value_t step_result_value = 0.0;
	explicit SolutionItem() {}
	explicit SolutionItem(auto idx) : idx(idx) {}
	explicit SolutionItem(auto idx, auto step_result_value) : idx(idx), step_result_value(step_result_value) {}
};

using Solution = std::vector<SolutionItem>;

std::optional<Solution> solve(math_value_t input_value, math_value_t desired_output, std::span<MathExpression::MathExpression> const expressions);
