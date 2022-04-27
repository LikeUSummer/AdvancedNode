#include <assert.h>
#include <node_api.h>

#include <iostream>
#include <vector>
#include <thread>

struct CallbackParam {
    int id {0};
    std::string message;
};
std::thread* g_threadList[5];

void OnSafeCall(napi_env env, napi_value callback, void* context, void* data)
{
    CallbackParam* callbackData = (CallbackParam*)data;
    napi_value param[2];
    napi_create_int32(env, callbackData->id, &param[0]);
    napi_create_string_utf8(env, callbackData->message.c_str(), callbackData->message.length(), &param[1]);

    napi_value undefined;
    napi_status status = napi_get_undefined(env, &undefined);
    napi_value result;
    status = napi_call_function(env, undefined, callback, 2, param, &result);
    assert(status == napi_ok);
}

void OnFinish(napi_env env, void* finalize_data, void* finalize_hint)
{
    for (auto& thread : g_threadList) {
        if (thread->joinable()) {
            thread->join();
        }
        delete thread;
    }
    std::cout << "finish";
}

void Task(napi_threadsafe_function safeCallback)
{
    // 通知引擎本线程将开始引用 safeCallback，增加引用计数，避免其他线程释放后计数归零、被 GC 回收
    napi_acquire_threadsafe_function(safeCallback); // 可注释掉实验，会发现运行异常，因为过程中 safeCallback 已被回收
    for (int i = 0; i < 5; ++i) {
        CallbackParam param {i, "event content"};
        napi_call_threadsafe_function(safeCallback, &param, napi_tsfn_blocking);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // 通知引擎本线程不再使用 safeCallback，将减少此函数的引用计数，计数归零并被回收时将触发 OnFinish（finalize callback）
    napi_release_threadsafe_function(safeCallback, napi_tsfn_release);
}

napi_value Register(napi_env env, napi_callback_info callbackInfo)
{
    size_t argc = 1;
    napi_value args[1];
    napi_status status = napi_get_cb_info(env, callbackInfo, &argc, args, NULL, NULL);
    assert((status == napi_ok) && (argc == 1));

    napi_value resourceName;
    status = napi_create_string_utf8(env, "testSafeCallback", NAPI_AUTO_LENGTH, &resourceName);
    assert(status == napi_ok);

    // 将普通 js 函数包装为安全函数，便于在多线程环境使用
    napi_threadsafe_function safeCallback;
    status = napi_create_threadsafe_function(env,
        args[0], // 普通 js 函数
        nullptr,
        resourceName,
        0,
        1,
        nullptr, // thread finalize data
        OnFinish,   // thread finalize callback
        nullptr,
        OnSafeCall,
        &safeCallback);
    assert(status == napi_ok);

    for (auto& thread : g_threadList) {
        thread = new std::thread(Task, safeCallback);
    }
    // 注意创建完成后，safeCallback 即有了一个引用计数，这里要释放掉，否则子线程完成计算，还残留一个计数
    napi_release_threadsafe_function(safeCallback, napi_tsfn_release); // 可注释掉实验，会发现任务结束后，OnFinish 未能触发
    return resourceName;
}

// 注册接口与初始化
#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }
napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("register", Register)
    };
    napi_status status = napi_define_properties(env, exports, 1, desc);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
