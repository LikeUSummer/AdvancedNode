/**
 * 东方盛夏 2022
 * https://www.zhihu.com/people/da-xia-tian-60
 */

class SPromise {
  static STATUS_PENDING = -1
  static STATUS_RESOLVED = 0
  static STATUS_REJECTED = 1

  constructor(callback) {
    if (typeof callback !== 'function') {
      throw new TypeError('Promise resolver is not a function')
    }
    // 定义 Promise 当前状态
    this.status = SPromise.STATUS_PENDING
    // 定义 Promise 值
    this.value = null
    // 定义 resolve/reject 时的承诺兑现回调
    this.onFulfill = [[], []]
    // Promise 创建即运行
    callback(this._genFulfiller(SPromise.STATUS_RESOLVED),
      this._genFulfiller(SPromise.STATUS_REJECTED)) 
  }

  then(onResolve, onReject) {
    return new SPromise((resolve, reject) => {
      function genCallback(fulfill, onFulfill) {
        return value => {
          if (typeof onFulfill === 'function') {
            let result = onFulfill(value)
            if (result instanceof SPromise) {
              // 若当前兑现回调仍返回 promise，则将它的兑现结果用来兑现当前 promise，由此可形成串联有序的 then 异步链
              result.then(resolve, reject)
            } else {
              fulfill(result)
            }
          } else {
            fulfill(value) // 当前 then 中无处理函数，直接将值传给下个 Promise 的 then
          }
        }
      }
      this.onFulfill[SPromise.STATUS_RESOLVED].push(genCallback(resolve, onResolve))
      this.onFulfill[SPromise.STATUS_REJECTED].push(genCallback(reject, onReject))
    })
  }

  _genFulfiller(status) {
    return value => {
      process.nextTick(() => { // 可以用 setImmediate，process.nextTick，setTimeout 等实验
        if (this.status !== SPromise.STATUS_PENDING) {
          return
        }
        this.status = status
        this.value = value
        let handler
        while (handler = this.onFulfill[status].shift()) {
          handler(this.value)
        }
      })
    }
  }
}

function test1() {
  let text = '界世，好你'
  return new SPromise((resolve, reject) => {
    setTimeout(() => {
      resolve(text.split(''))
    },1000)
  }).then((value) => { // 异步回调
    console.log('[then1]', value)
    return new SPromise((resolve, reject) => {
      setTimeout(() => {
        resolve(value.reverse())
      })
    })
  }).then((value) => { // 同步回调
    console.log('[then2]', value)
    return value.join('')
  }).then((value) => {
    console.log('[then3]', value)
  })
}

function test2() {
  const https = require('https')
  function get(keyword) {
    return new SPromise((resolve, reject) => {
      https.get('https://suggest.taobao.com/sug?code=utf-8&q=' + keyword, response => {
        let data = ''
        response.on('data', chunk => {
          data += chunk
        });
        response.on('end', () => {
          resolve(data)
        });
      }).on('error', err => {
        reject(err.message)
      })
    })
  }

  get('华为')
  .then((value) => { // 异步回调
    console.log('[then1]', value)
    return get('苹果')
  }).then((value) => { // 同步回调
    console.log('[then2]', value)
    return JSON.parse(value)
  }).then((value) => {
    console.log('[then3]', value['result'])
  })
}

test1()
.then(() => {
  console.log('---------------------------')
  test2()
})
