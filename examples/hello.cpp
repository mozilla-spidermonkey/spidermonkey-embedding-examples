#include <cstdio>

#include <js/Initialization.h>
#include <jsapi.h>

/* This example illustrates the bare minimum you need to do to execute a
 * JavaScript program using embedded SpiderMonkey. It does no error handling and
 * simply exits if something goes wrong.
 *
 * To use the interpreter you need to create a context and a global object, and
 * do some setup on both of these. You also need to enter a "request" (lock on
 * the interpreter) and a "compartment" (environment within one global object)
 * before you can execute code. */

static JSClassOps globalOps = {nullptr,  // addProperty
                               nullptr,  // deleteProperty
                               nullptr,  // enumerate
                               nullptr,  // newEnumerate
                               nullptr,  // resolve
                               nullptr,  // mayResolve
                               nullptr,  // finalize
                               nullptr,  // call
                               nullptr,  // hasInstance
                               nullptr,  // construct
                               JS_GlobalObjectTraceHook};

// The class of the global object.
static JSClass globalClass = {"HelloWorldGlobal", JSCLASS_GLOBAL_FLAGS,
                              &globalOps};

static JSContext* CreateContext(void) {
  JSContext* cx = JS_NewContext(8L * 1024 * 1024);
  if (!cx) return nullptr;
  if (!JS::InitSelfHostedCode(cx)) return nullptr;
  return cx;
}

static JSObject* CreateGlobal(JSContext* cx) {
  JS::CompartmentOptions options;
  JS::RootedObject global(
      cx, JS_NewGlobalObject(cx, &globalClass, nullptr, JS::FireOnNewGlobalHook,
                             options));

  // Add standard JavaScript classes to the global so we have a useful
  // environment.
  JSAutoCompartment ac(cx, global);
  if (!JS_InitStandardClasses(cx, global)) return nullptr;

  return global;
}

static bool ExecuteCodePrintResult(JSContext* cx, const char* code) {
  JS::CompileOptions options(cx);
  options.setFileAndLine("noname", 1);
  JS::RootedValue rval(cx);
  if (!JS::Evaluate(cx, options, code, strlen(code), &rval)) return false;

  // There are many ways to display an arbitrary value as a result. In this
  // case, we know that the value is a string because of the expression that we
  // executed, so we can just print the string directly.
  printf("%s\n", JS_EncodeString(cx, rval.toString()));
  return true;
}

static bool Run(JSContext* cx) {
  JSAutoRequest ar(cx);

  JS::RootedObject global(cx, CreateGlobal(cx));
  if (!global) return false;

  JSAutoCompartment ac(cx, global);

  // The 'js' delimiter is meaningless, but it's useful for marking C++ raw
  // strings semantically.
  return ExecuteCodePrintResult(cx, R"js(
    `hello world, it is ${new Date()}`
  )js");
}

int main(int argc, const char* argv[]) {
  if (!JS_Init()) return 1;

  JSContext* cx = CreateContext();
  if (!cx) return 1;

  if (!Run(cx)) return 1;

  JS_DestroyContext(cx);
  JS_ShutDown();
  return 0;
}
