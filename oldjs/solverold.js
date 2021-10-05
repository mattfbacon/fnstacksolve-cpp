const mathjs = require('mathjs');

const f = x => (-x-2)**2;
const g = x => -(x/2)-10;
const h = x => -Math.abs(3*x);
const i = x => -(x/2)+34;

const a = [f, g, h, i];
const proper = {
  f: '(-x-2)²',
  g: '-(x/2)-10',
  h: '-|3x|',
  i: '-(x/2)+34'
}

outer:
for (var j = 0; j < a.length; j++) {
  const temp1 = a[j](0);
  const a2 = a.concat();
  a2.splice(j, 1);
  for (var k = 0; k < a2.length; k++) {
    const temp2 = a2[k](temp1);
    const a3 = a2.concat();
    a3.splice(k, 1);
    for (var l = 0; l < a3.length; l++) {
      const temp3 = a3[l](temp2);
      const a4 = a3.concat()
      a4.splice(l, 1);
      if (a4[0](temp3) == -6) { process.stdout.write(`${proper[a[j].name]}  ->  ${proper[a2[k].name]}  ->  ${proper[a3[l].name]}  ->  ${proper[a4[0].name]}  ->  ${a4[0](temp3)}\n`); break outer; }
    }
  }
}
