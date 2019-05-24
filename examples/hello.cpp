#include <cstdio>

#include <jsapi.h>

#include "boilerplate.h"

// This example illustrates the bare minimum you need to do to execute a
// JavaScript program using embedded SpiderMonkey. It does no error handling and
// simply exits if something goes wrong.
//
// See 'boilerplate.cpp' for the parts of this example that are reused in many
// simple embedding examples.
//
// To use the interpreter you need to create a context and a global object, and
// do some setup on both of these. You also need to enter a "compartment"
// (environment within one global object) before you can execute code.

static bool ExecuteCodePrintResult(JSContext* cx, const char* code) {
  JS::CompileOptions options(cx);
  options.setFileAndLine("noname", 1);
  JS::RootedValue rval(cx);
  if (!JS::Evaluate(cx, options, code, strlen(code), &rval)) {
    return false;
  }

  // There are many ways to display an arbitrary value as a result. In this
  // case, we know that the value is a string because of the expression that we
  // executed, so we can just print the string directly.
  printf("%s\n", JS_EncodeString(cx, rval.toString()));
  return true;
}

static bool HelloExample(JSContext* cx) {
  JS::RootedObject global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    return false;
  }

  JSAutoCompartment ac(cx, global);

  // The 'js' delimiter is meaningless, but it's useful for marking C++ raw
  // strings semantically.
  return ExecuteCodePrintResult(cx, R"js(
    `hello world, it is ${new Date()}`
  )js");
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(HelloExample)) {
    return 1;
  }
  return 0;
}
