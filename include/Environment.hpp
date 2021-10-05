#pragma once

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "WrappedFunction.hpp"

#include "common.hpp"

class Environment {
public:
	using key_t = std::string;  // would like to use string_view but PR0919r3 has not been implemented...
	using variable_value_t = math_value_t;
	using function_value_t = WrappedFunction;
protected:
	std::unordered_map<key_t, variable_value_t> variables;
	std::unordered_map<key_t, function_value_t> functions;
public:
	Environment() {}
	Environment(std::initializer_list<decltype(variables)::value_type> variables, std::initializer_list<decltype(functions)::value_type> functions)
		: variables(std::move(variables)), functions(std::move(functions)) {}
	static Environment default_environment(variable_value_t const independent);

	bool has_variable(key_t const key) const {
		return variables.contains(key);
	}
	bool has_function(key_t const key) const {
		return functions.contains(key);
	}

	std::optional<variable_value_t> get_variable(key_t const key) const {
		try {
			return { variables.at(key) };
		} catch (std::out_of_range const& e) {
			return {};
		}
	}
	std::optional<std::reference_wrapper<function_value_t const>> get_function(key_t const key) const {
		try {
			return { functions.at(key) };
		} catch (std::out_of_range const& e) {
			return {};
		}
	}

	void set_variable(key_t const key, variable_value_t const& value) {
		variables[key] = value;
	}
	void set_function(key_t const key, function_value_t const& value) {
		functions[key] = value;
	}

	void clear_variable(key_t const key) {
		variables.erase(key);
	}
	void clear_function(key_t const key) {
		functions.erase(key);
	}

	void set_independent(variable_value_t const& value) {
		set_variable("x", value);
	}
};
