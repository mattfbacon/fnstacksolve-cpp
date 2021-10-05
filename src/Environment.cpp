#include "Environment.hpp"
#include "MathExpression.hpp"

math_value_t nthrt(math_value_t const n, math_value_t const value) {
	return ::pow(value, (math_value_t)1.0 / n);
}
math_value_t csc(math_value_t const x) {
	return (math_value_t)1.0 / ::sin(x);
}
math_value_t sec(math_value_t const x) {
	return (math_value_t)1.0 / ::cos(x);
}
math_value_t cot(math_value_t const x) {
	return (math_value_t)1.0 / ::tan(x);
}
math_value_t logn(math_value_t const n, math_value_t const value) {
	return ::log(value) / ::log(n);
}

Environment Environment::default_environment(variable_value_t const independent) {
	auto ret = Environment(
		{
			{ "x", independent },
		},
		{
			{ "mod", WrappedFunction::create<2>(fmod) },
			{ "pow", WrappedFunction::create<2>(pow) },
			{ "abs", WrappedFunction::create<1>(fabs) },
			{ "sqrt", WrappedFunction::create<1>(sqrt) },
			{ "cbrt", WrappedFunction::create<1>(cbrt) },
			// n, value -> ⁿ√v̅a̅l̅u̅e̅
			{ "nthrt", WrappedFunction::create<2>(nthrt) },
			{ "sin", WrappedFunction::create<1>(sin) },
			{ "cos", WrappedFunction::create<1>(cos) },
			{ "tan", WrappedFunction::create<1>(tan) },
			{ "csc", WrappedFunction::create<1>(csc) },
			{ "sec", WrappedFunction::create<1>(sec) },
			// n, value -> logₙ(value)
			{ "cot", WrappedFunction::create<1>(cot) },
			{ "logn", WrappedFunction::create<2>(logn) },
			{ "ln", WrappedFunction::create<1>(log) },
			{ "log", WrappedFunction::create<1>(log10) },
		});
	return ret;
}
