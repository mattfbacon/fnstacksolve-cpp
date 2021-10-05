# Function Stack Solver (C++)

Reimplementation of a function stack solver (originally available on my website, and now in `oldjs/`) in C++.

Evaluates permutations of the expressions in order to find the order that gets a given output from a given input.

- Parser written using PEG grammar
	- Functions
	- Variables
	- Left- and right-associative operators
	- Positive and negative decimal numbers
		- Fuzzy equals checking to mitigate floating point issues
	- Evaluation of resulting parse tree based on dynamic environments

## Build and Run

```bash
make debug # or release
./dist/Debug/fnstacksolve # or Release
# input value: 0
# output value: 8
# 1> x * 4
# 2> x + 2
# Solving...
# Solution:
# 0> Starting with 0
# 2> x + 2 => 2
# 1> x * 4 => 8
```
