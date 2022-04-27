#include <assert.h>
#include <node_api.h>

static napi_value Register(napi_env env, const napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1];
    napi_status status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
    assert(status == napi_ok);

    napi_value callback = argv[0];

    napi_value params[1];
    status = napi_create_string_utf8(env, "hello world", NAPI_AUTO_LENGTH, params);
    assert(status == napi_ok);

    napi_value global;
    status = napi_get_global(env, &global);
    assert(status == napi_ok);

    napi_value result;
    status = napi_call_function(env, global, callback, 1, params, &result);
    assert(status == napi_ok);
    return nullptr;
}

static napi_value Init(napi_env env, napi_value exports) {
    napi_value new_exports;
    napi_status status = napi_create_function(env, "", NAPI_AUTO_LENGTH,
        Register, NULL, &new_exports);
    assert(status == napi_ok);
    return new_exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
