#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "tao/pegtl.hpp"

#include "MathExpression.hpp"

#include "solve.hpp"

int main() {
	std::vector<MathExpression::MathExpression> expressions;
	MathExpression::value_t input_value, desired_output;

	std::cout << "Input value: " << std::flush;
	std::cin >> input_value;

	std::cout << "Desired output value: " << std::flush;
	std::cin >> desired_output;

	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << "Input the expressions with x as the independent:" << std::endl;
	for (size_t expression_index = 0; expression_index < std::numeric_limits<size_t>::max(); expression_index++) {
try_again:;
		std::string storage;
		std::cout << (expression_index + 1) << "> " << std::flush;
		std::getline(std::cin, storage);
		if (!std::cin.good() || storage.empty()) {
			break;
		}
		if (expressions.size() > 0) std::cout << expressions[0].evaluate(Environment::default_environment(input_value)) << std::endl;
		try {
			// parse from string
			expressions.emplace_back(std::move(storage));
		} catch (tao::pegtl::parse_error const& e) {
			std::clog << e.message() << '\n';
			if (e.positions().size() > 0) {
				std::clog << "At column";
				if (e.positions().size() != 1) std::clog << 's';
				std::clog << " ";
				for (auto const& position : e.positions()) {
					std::clog << position.column << ", ";
				}
				std::clog << std::endl;
			}
			goto try_again;
		}
		if (expressions.size() > 0) std::cout << expressions[0].evaluate(Environment::default_environment(input_value)) << std::endl;
	}

	if (expressions.size() == 0) {
		std::clog << "No expressions given." << std::endl;
		return 0;
	}

	std::cout << "Solving..." << std::endl;
	// using Solution = std::vector<size_t> representing indices in `expressions`
	auto const solution_ = solve(input_value, desired_output, expressions);
	if (!solution_.has_value()) {
		std::cout << "No solution found." << std::endl;
		return 2;
	}
	std::cout << "Solution:\n";
	std::cout << "0> Starting with " << input_value << '\n';

	for (auto const [expression_index, value_at_step] : *solution_) {
		auto const& expression = expressions[expression_index];
		std::cout << (expression_index + 1) << "> " << expression.get_original_text() << " => " << value_at_step << std::endl;
	}
	return 0;
}
