function toOrdinal(number) {
  const str = number.toString();
  const lastTwoDigits = Math.abs(number % 100);
  const lastChar = str.charAt(str.length - 1);
  return str + ((lastTwoDigits >= 11 && lastTwoDigits <= 13) ? 'th'
          : lastChar === '1' ? 'st'
          : lastChar === '2' ? 'nd'
          : lastChar === '3' ? 'rd'
          : 'th');
}

function pure_splice(array, start = 0, deleteCount = 0, ...items) {
  const arrayLength = array.length;
  const _deleteCount = (deleteCount < 0) ? 0 : deleteCount;
  let _start;
  if (start < 0) {
    if (Math.abs(start) > arrayLength) {
      _start = 0;
    } else {
      _start = arrayLength + start;
    }
  } else if (start > arrayLength) {
    _start = arrayLength;
  } else {
    _start = start;
  }
  return [
    ...array.slice(0, _start),
    ...items,
    ...array.slice((_start + _deleteCount), arrayLength),
  ];
}

function solve (n, eqstack, stacksofar, solution) {
  if (eqstack.length === 1) {
		console.info(eqstack, n, eqstack[0].compile().evaluate({x:n}));
    if (eqstack[0].compile().evaluate({x:n}) === solution) return stacksofar.concat(eqstack[0]);
    else return false;
  } else {
    for (let i = 0; i < eqstack.length; i++) {
			console.info('loop', eqstack, n);
      const thisoutput = solve(eqstack[i].compile().evaluate({x:n}), pure_splice(eqstack, i, 1), stacksofar.concat(eqstack[i]), solution);
      if (thisoutput) return thisoutput;
    }
  }
};

const Solver = {
	el: '#vue-mount',
	data() {
		return {
			inputValue: 0,
			outputValue: 0,
			functions: [],
			result: 'Click Solve.'
		}
	},
	components: {
		'function-entry': {
			data() {
				return {
					myfn: this.fn
				}
			},
			computed: {
				itemId() {
					return `equation-${this.index}`;
				},
				ordinalIndex() {
					return toOrdinal(this.index+1);
				}
			},
			methods: {
				removeEquation() {
					this.$emit('remove-fn', this.index);
				}
			},
			watch: {
				fn(val) {
					this.myfn = val;
				},
				myfn(val) {
					this.$emit('update-fn', this.index, val);
				}
			},
			props: {
				index: Number,
				fn: String
			},
			template: '<li><input type="text" v-model="myfn"><button @click="removeEquation">&times;</button></li>'
		}
	},
	methods: {
		removeFunction(index) {
			this.functions.splice(index, 1);
		},
		addFunction() {
			this.functions.push('');
		},
		updateFunction(index, val) {
			this.functions[index] = val;
		},
		solve() {
			const parsedFunctions = this.functions.map(x=>math.parse(x));
			console.info(parsedFunctions);
			const solution = solve(this.inputValue, parsedFunctions, [], this.outputValue);
			if (typeof solution == 'undefined') {
				this.result = 'No solution';
			} else {
				this.result = solution.map(x=>x.toString()).join('  ->  ');
			}
		}
	}
}

const app = new Vue(Solver);