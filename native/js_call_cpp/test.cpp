#include <assert.h>
#include <node_api.h>

#include <string>

static napi_value Hello(napi_env env, napi_callback_info info)
{
    napi_value nodeString;
    std::string text = "hello, world";
    napi_status status = napi_create_string_utf8(env,
        text.c_str(), text.length(), &nodeString);
    assert(status == napi_ok);
    return nodeString;
}

napi_value Add(napi_env env, napi_callback_info info)
{
    // 获取传入参数信息
    size_t argc = 2;
    napi_value argv[2];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    assert(status == napi_ok);
    if (argc < 2) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments.");
        return NULL;
    }

    // 校验参数类型
    napi_valuetype valueType1, valueType2;
    status = napi_typeof(env, argv[0], &valueType1);
    assert(status == napi_ok);
    status = napi_typeof(env, argv[1], &valueType2);
    assert(status == napi_ok);
    if (valueType1 != napi_number || valueType2 != napi_number) {
        napi_throw_type_error(env, NULL, "Wrong arguments");
        return NULL;
    }

    // 获取参数值
    double value1, value2;
    status = napi_get_value_double(env, argv[0], &value1);
    assert(status == napi_ok);
    status = napi_get_value_double(env, argv[1], &value2);
    assert(status == napi_ok);

    // 计算并返回
    napi_value sum;
    status = napi_create_double(env, value1 + value2, &sum);
    assert(status == napi_ok);
    return sum;
}

// 注册接口与初始化
#define DECLARE_NAPI_METHOD(name, func) { name, 0, func, 0, 0, 0, napi_default, 0 }

static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_METHOD("hello", Hello),
        DECLARE_NAPI_METHOD("add", Add)
    };
    napi_status status = napi_define_properties(env, exports, 2, desc);
    assert(status == napi_ok);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
