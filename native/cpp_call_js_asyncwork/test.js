const { register } = require('./out/test.node');

function onEvent(id, message) {
    console.log(id, message)
}

function onFinish(text) {
    console.log(text)
}

register(onEvent, onFinish);
