/**
 * node.js 异步 IO 特殊场景，有助于深入理解事件循环与异步调用过程
 * 
 * author: 东方盛夏
 * date: 2022-04-22
 */
let fs = require('fs')
let util = require('util')

let readFile = util.promisify(fs.readFile);
let writeFile = util.promisify(fs.writeFile);

if (!fs.existsSync('data.txt')) {
    let text = 'x';
    for (let i = 0; i < 12; ++i) { // 生成 4KB 数据
        text += text;
    }
    fs.writeFileSync('data.txt', text);
}

/**
 * 返回情况1：返回大于 1 的数字
 * 解释1：首次调用的异步 readFile 任务尚未完成，但定时器已再次触发，count++ 被载入虚拟机执行多次
 * 
 * 返回情况2：返回 1
 * 解释2：首次调用的异步 readFile 任务在定时周期内完成，虽然后续同步代码耗时大于 1 秒，
 * 但由于它们独占虚拟机（单线程），下个定时器即便已到期，也没有机会加载执行，直到当前同步代码块执行完。
 * 
 * 注意事项：作者在实验时，data.txt 大小为 4KB，定时器周期为 2 毫秒，但读者在运行时，
 * 可能需要根据自己的机器配置调整这两个参数，才能同时看到上述两种随机结果，否则只会有一种结果。
 */
async function test1() {
    return new Promise(async (resolve, reject) => {
      let count = 0;
      let handle = setInterval(async () => {
        count++;
        let data = await readFile('data.txt', { encoding: 'utf-8' });
        // 执行同步代码，耗时远大于 2 毫秒
        let delay1 = 60000;
        while (--delay1) {
            let delay2 = delay1;
            while (--delay2);
        }
        resolve(count);
        clearInterval(handle);
      }, 2); // 高频触发
    });
}

/**
 * 返回情况1：两个数字都是 50
 * 解释1：异步任务 writeFile 与 readFile 都在一个定时周期内完成。
 * 
 * 返回情况2：第一个数字大于 50，第二个数字是 50
 * 解释2：异步任务 readFile 在执行过程中，定时器已再次触发，count++ 被重复执行，
 * await writeFile 也可能正被重复解释执行，但 writeFile 异步任务还未有效工作。
 * 
 * 返回情况3：两个大于 100 的数字
 * 解释3：异步任务 readFile 在执行过程中，定时器已再次触发，异步任务 writeFile
 * 被重复执行并有效工作（改变了文件内容），则之前触发的尚未完成的 readFile 任务
 * 将读到预期外的文件内容，行首找不到 50（它对应的 writeFile 在行首写下了 50），
 * 只能靠条件 count > 100 来结束定时。这说明过程中不止有一个 writeFile 任务在
 * 完成并回调时，之前写的内容已被新的异步 writeFile 所覆盖。
 * 
 * 返回情况4：只有一个数字
 * 解释4：在解释3描述的过程中，还有一个特殊情况是，readFile 在执行过程中，后续
 * 定时器中的 writeFile 刚锁住或擦除了文件内容，则此 readFile 将读不到任何内容，
 * 所以第二个数字是空的。
 * 
 */
async function test2() {
    let data = await readFile('data.txt', { encoding: 'utf-8' });
    return new Promise(async (resolve, reject) => {
      let filePath = 'test.log';
      let count = 0;
      let handle = setInterval(async () => {
        count++;
        await writeFile(filePath, count + '\n' + data);
        let text = await readFile(filePath, { encoding: 'utf-8' });
        if (text.substring(0, 3) == '50\n' || count > 100) {
            resolve(count + ' ' + text.substring(0, text.search('\n')));
            clearInterval(handle);
        }
      }, 2); // 高频触发
    });
}

(async () => {
    console.log('test1');
    for (let i = 0; i < 10; ++i) {
        let text = await test1();
        console.log(text);
    }
    console.log('test2');
    for (let i = 0; i < 10; ++i) {
        let text = await test2();
        console.log(text);
    }
})();
