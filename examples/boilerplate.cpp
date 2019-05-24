#include <jsapi.h>

#include <js/Initialization.h>

#include "boilerplate.h"

// This file contains boilerplate code used by a number of examples. Ideally
// this should eventually become part of SpiderMonkey itself.

// A standard set of ClassOps for globals. This includes hooks to resolve
// standard JavaScript builtin types to give a more full-featured shell.
const JSClassOps boilerplate::DefaultGlobalClassOps = {
    nullptr,                         // addProperty
    nullptr,                         // deleteProperty
    nullptr,                         // enumerate
    JS_NewEnumerateStandardClasses,  // newEnumerate
    JS_ResolveStandardClass,         // resolve
    JS_MayResolveStandardClass,      // mayResolve
    nullptr,                         // finalize
    nullptr,                         // call
    nullptr,                         // hasInstance
    nullptr,                         // construct
    JS_GlobalObjectTraceHook         // trace
};

// Create a simple Global object. A global object is the top-level 'this' value
// in a script and is required in order to compile or execute JavaScript.
JSObject* boilerplate::CreateGlobal(JSContext* cx) {
  JS::RealmOptions options;

  static JSClass BoilerplateGlobalClass = {"BoilerplateGlobal",
                                           JSCLASS_GLOBAL_FLAGS,
                                           &boilerplate::DefaultGlobalClassOps};

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
