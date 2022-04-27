#include <assert.h>
#include <node_api.h>

#include <iostream>
#include <thread>

struct TaskData {
    std::string result;
    napi_ref normalCallback {nullptr};
    napi_threadsafe_function safeCallback {nullptr};
    napi_async_work asyncTask {nullptr};
};

struct CallbackData {
    int id {0};
    std::string message;
};

void SafeCallbackProxy(napi_env env, napi_value callback, void* context, void* data)
{
    CallbackData* callbackData = (CallbackData*)data;
    napi_value param[2];
    napi_create_int32(env, callbackData->id, &param[0]);
    napi_create_string_utf8(env, callbackData->message.c_str(), callbackData->message.length(), &param[1]);

    napi_value result;
    napi_value undefined;
    napi_status status = napi_get_undefined(env, &undefined);
    status = napi_call_function(env, undefined, callback, 2, param, &result);
    assert(status == napi_ok);
}

void Task(napi_env env, void* data)
{
    TaskData* taskData = (TaskData*)data;
    for (int i = 0; i < 10; ++i) {
        CallbackData param {i, "event content"};
        napi_call_threadsafe_function(taskData->safeCallback, &param, napi_tsfn_blocking);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    taskData->result = "done";
}

void OnFinish(napi_env env, napi_status status, void* data)
{
    TaskData* taskData = (TaskData*)data;
    if (status == napi_cancelled) {
        std::cout << "cancel\n";
    } else if (status == napi_ok) {
        napi_value callback;
        napi_value global;
        napi_value result;
        napi_value param;
        napi_create_string_utf8(env, taskData->result.c_str(), taskData->result.length(), &param);
        napi_get_reference_value(env, taskData->normalCallback, &callback);
        napi_get_global(env, &global);
        napi_call_function(env, global, callback, 1, &param, &result);

        napi_delete_reference(env, taskData->normalCallback);
        napi_release_threadsafe_function(taskData->safeCallback, napi_tsfn_release);
        napi_delete_async_work(env, taskData->asyncTask);
    }
    delete taskData;
}

napi_value Register(napi_env env, napi_callback_info callbackInfo)
{
    size_t argc = 2;
    napi_value args[2];
    napi_status status = napi_get_cb_info(env, callbackInfo, &argc, args, NULL, NULL);
    assert((status == napi_ok) && (argc == 2));

    napi_value resourceName;
    status = napi_create_string_utf8(env, "test", NAPI_AUTO_LENGTH, &resourceName);
    assert(status == napi_ok);

    TaskData* taskData = new TaskData();
    // 对多线程回调函数创建安全函数实例
    status = napi_create_threadsafe_function(env,
        args[0], // js 回调函数
        nullptr,
        resourceName,
        0,
        1,
        nullptr,
        nullptr,
        nullptr,
        SafeCallbackProxy, // 实际执行入口
        &taskData->safeCallback); // 安全化后的回调函数
    assert(status == napi_ok);
    // 对常规函数创建引用
    status = napi_create_reference(env, args[1], 1, &taskData->normalCallback);
    assert(status == napi_ok);

    // 给 uv 线程池添加任务
    status = napi_create_async_work(env, nullptr, resourceName,
        Task,
        OnFinish,
        (void*)taskData,
        &taskData->asyncTask);
    assert(status == napi_ok);
    status = napi_queue_async_work(env, taskData->asyncTask);

    napi_value ret;
    napi_create_int32(env, 0, &ret);
    return ret;
}

napi_value Init(napi_env env, napi_value exports)
{
    napi_value func;
    napi_create_function(env,
        NULL,
        NAPI_AUTO_LENGTH,
        Register,
        NULL,
        &func);
    napi_set_named_property(env, exports, "register", func);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
