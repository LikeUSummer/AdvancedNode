let addon = require('./out/test');

console.log(addon.hello());
console.log(addon.add(3.14, 1.42).toFixed(2));
