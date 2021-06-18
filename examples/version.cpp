#include <cassert>
#include <iostream>
#include <cstdio>

#include <jsapi.h>
#include <js/CompilationAndEvaluation.h>
#include <js/SourceText.h>

#include "boilerplate.h"

// Return JavaScript version
static bool version(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  const char* data = JS_GetImplementationVersion();
  JSString* str = JS_NewStringCopyZ(cx, data);
  args.rval().setString(str);
  return true;
}

// Copy from hello.cpp
static bool ExecuteCodePrintResult(JSContext* cx, const char* code) {
  JS::CompileOptions options(cx);
  options.setFileAndLine("noname", 1);

  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(cx, code, strlen(code), JS::SourceOwnership::Borrowed)) {
    return false;
  }

  JS::RootedValue rval(cx);
  if (!JS::Evaluate(cx, options, source, &rval)) return false;

  printf("%s\n", JS_EncodeStringToASCII(cx, rval.toString()).get());
  return true;
}

static bool RunVersion(JSContext* cx) {
  JS::RootedObject global(cx, boilerplate::CreateGlobal(cx));
  if (!global) return false;

  JSAutoRealm ar(cx, global);

  // Define some helper methods on our new global.
  if (!JS_DefineFunction(cx, global, "version", version, 0, 0)) return false;

  return ExecuteCodePrintResult(cx, R"js(
    version()
  )js");
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(RunVersion)) return 1;
  return 0;
}
