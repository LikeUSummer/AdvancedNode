const { register } = require('./out/test.node');

function onEvent(id, message) {
    console.log(id, message)
}

console.log(register(onEvent));
