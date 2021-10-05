const mathjs = require('mathjs');
const readline = require('readline');
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

const read = readline.createInterface({ input: process.stdin, output: process.stdout });

/**
 * A pure version of Array.prototype.splice
 * It will return a new array rather than mutate the array
 * See: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Array/splice
 * @param {Array} array The target array
 * @param {number} start Index at which to start changing the array
 * @param {number} deleteCount An integer indicating the number of old array elements to remove
 * @param {any} items The elements to add to the array, beginning at the start index
 * @returns {Array}
 */
function pure_splice(array, start = 0, deleteCount = 0, ...items) {
  const arrayLength = array.length
  const _deleteCount = (deleteCount < 0) ? 0 : deleteCount
  let _start
  if (start < 0) {
    if (Math.abs(start) > arrayLength) {
      _start = 0
    } else {
      _start = arrayLength + start
    }
  } else if (start > arrayLength) {
    _start = arrayLength
  } else {
    _start = start
  }
  return [
    ...array.slice(0, _start),
    ...items,
    ...array.slice((_start + _deleteCount), arrayLength),
  ]
}

const solve = (n, eqstack, stacksofar, solution) => {
  if (eqstack.length === 1) {
    if (eqstack[0].compile().evaluate({x:n}) === solution) return stacksofar.concat(eqstack[0]);
    else return false;
  } else {
    for (let i = 0; i < eqstack.length; i++) {
      const thisoutput = solve(eqstack[i].compile().evaluate({x:n}), pure_splice(eqstack, i, 1), stacksofar.concat(eqstack[i]), solution);
      if (thisoutput) return thisoutput;
    }
  }
};

(async () => {
  const exprs = [];

  process.stdout.write('to put in |x|, write abs(x). to put in √x, type sqrt(x).\n');

  while (true) {
    const value = await new Promise(resolve => read.question(`${toOrdinal(exprs.length+1)} function (enter to finish): `, resolve));
    if (value === '') {
      break;
    } else {
      exprs.push(mathjs.parse(value));
    }
  }

  if (exprs.length == 0) {
    process.stdout.write('No functions given. Exiting...\n');
    process.exit(2);
  }

  const input = parseInt(await new Promise(resolve => read.question('Input value: ', resolve)));
  const output = parseInt(await new Promise(resolve => read.question('Desired output value: ', resolve)));

  const result = solve(input, exprs, [], output);

  if (result) { process.stdout.write(result.map(x=>x.toString()).join('  ->  ')+'  ->  '+output+'\n'); process.exit(0); }
  else { process.stdout.write(`No solution for output ${output} with input ${input}`); process.exit(1); }
})();