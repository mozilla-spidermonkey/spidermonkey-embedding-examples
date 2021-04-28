#include <jsapi.h>

#include <js/Initialization.h>
#include <js/Exception.h>

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

// Helper to read current exception and dump to stderr.
//
// NOTE: This must be called with a JSAutoRealm (or equivalent) on the stack.
void boilerplate::ReportAndClearException(JSContext* cx) {
  JS::ExceptionStack stack(cx);
  if (!JS::StealPendingExceptionStack(cx, &stack)) {
    fprintf(stderr, "Uncatchable exception thrown, out of memory or something");
    exit(1);
  }

  JS::ErrorReportBuilder report(cx);
  if (!report.init(cx, stack, JS::ErrorReportBuilder::WithSideEffects)) {
    fprintf(stderr, "Couldn't build error report");
    exit(1);
  }

  JS::PrintError(cx, stderr, report, false);
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
