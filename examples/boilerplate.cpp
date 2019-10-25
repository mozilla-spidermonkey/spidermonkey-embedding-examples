#include <jsapi.h>

#include <js/Initialization.h>

#include "boilerplate.h"

// This file contains boilerplate code used by a number of examples. Ideally
// this should eventually become part of SpiderMonkey itself.

// Create a simple Global object. A global object is the top-level 'this' value
// in a script and is required in order to compile or execute JavaScript.
JSObject* boilerplate::CreateGlobal(JSContext* cx) {
  JS::RealmOptions options;

  static JSClass BoilerplateGlobalClass = {
      "BoilerplateGlobal", JSCLASS_GLOBAL_FLAGS, &JS::DefaultGlobalClassOps};

  return JS_NewGlobalObject(cx, &BoilerplateGlobalClass, nullptr,
                            JS::FireOnNewGlobalHook, options);
}

// Initialize the JS environment, create a JSContext and run the example
// function in that context. By default the self-hosting environment is
// initialized as it is needed to run any JavaScript). If the 'initSelfHosting'
// argument is false, we will not initialize self-hosting and instead leave
// that to the caller.
bool boilerplate::RunExample(bool (*task)(JSContext*), bool initSelfHosting) {
  if (!JS_Init()) {
    return false;
  }

  JSContext* cx = JS_NewContext(JS::DefaultHeapMaxBytes);
  if (!cx) {
    return false;
  }

  if (initSelfHosting && !JS::InitSelfHostedCode(cx)) {
    return false;
  }

  if (!task(cx)) {
    return false;
  }

  JS_DestroyContext(cx);
  JS_ShutDown();

  return true;
}
