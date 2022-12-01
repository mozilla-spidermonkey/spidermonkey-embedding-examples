#include <cstdio>
#include <cstdint>
#include <chrono>
#include <thread>

#include <jsapi.h>
#include <js/CompilationAndEvaluation.h>
#include <js/Conversions.h>
#include <js/Initialization.h>
#include <js/SourceText.h>

#include "boilerplate.h"

// This example illustrates usage of SpiderMonkey in multiple threads. It does
// no error handling and simply exits if something goes wrong.
//
// See 'boilerplate.cpp' for the parts of this example that are reused in many
// simple embedding examples.
//
// To use SpiderMonkey API in multiple threads, you need to create a JSContext
// in the thread, using the main thread's JSRuntime as a parent, and initialize
// self-hosted code, and create its own global.

static bool ExecuteCode(JSContext* cx, const char* code) {
  JS::CompileOptions options(cx);
  options.setFileAndLine("noname", 1);

  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(cx, code, strlen(code), JS::SourceOwnership::Borrowed)) {
    return false;
  }

  JS::Rooted<JS::Value> rval(cx);
  if (!JS::Evaluate(cx, options, source, &rval)) {
    return false;
  }

  return true;
}

static bool Print(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS::Rooted<JS::Value> arg(cx, args.get(0));
  JS::Rooted<JSString*> str(cx, JS::ToString(cx, arg));
  if (!str) {
    return false;
  }

  JS::UniqueChars chars = JS_EncodeStringToUTF8(cx, str);
  fprintf(stderr, "%s\n", chars.get());

  args.rval().setUndefined();
  return true;
}

static bool Sleep(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS::Rooted<JS::Value> arg(cx, args.get(0));
  int32_t ms;
  if (!JS::ToInt32(cx, arg, &ms)) {
    return false;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(ms));

  args.rval().setUndefined();
  return true;
}

bool DefineFunctions(JSContext* cx, JS::Handle<JSObject*> global) {
  if (!JS_DefineFunction(cx, global, "print", &Print, 0, 0)) {
    return false;
  }
  if (!JS_DefineFunction(cx, global, "sleep", &Sleep, 0, 0)) {
    return false;
  }

  return true;
}

static void* WorkerMain(void* arg) {
  JSRuntime* parentRuntime = reinterpret_cast<JSRuntime*>(arg);

  JSContext* cx = JS_NewContext(8L * 1024L * 1024L, parentRuntime);

  if (!JS::InitSelfHostedCode(cx)) {
    return nullptr;
  }

  {
    JS::Rooted<JSObject*> global(cx, boilerplate::CreateGlobal(cx));
    if (!global) {
      return nullptr;
    }

    JSAutoRealm ar(cx, global);

    if (!DefineFunctions(cx, global)) {
      boilerplate::ReportAndClearException(cx);
      return nullptr;
    }

    if (!ExecuteCode(cx, R"js(
for (let i = 0; i < 10; i++) {
  print(`in worker thread, it is ${new Date()}`);
  sleep(1000);
}
    )js")) {
      boilerplate::ReportAndClearException(cx);
      return nullptr;
    }
  }

  JS_DestroyContext(cx);

  return nullptr;
}

static bool CreateWorker(JSContext* cx, pthread_t* thread) {
  if (pthread_create(thread, nullptr, WorkerMain, JS_GetRuntime(cx)) != 0) {
    return false;
  }

  return true;
}

static bool WorkerExample(JSContext* cx) {
  JS::Rooted<JSObject*> global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    return false;
  }

  pthread_t thread1;
  pthread_t thread2;

  if (!CreateWorker(cx, &thread1)) {
    return false;
  }
  if (!CreateWorker(cx, &thread2)) {
    return false;
  }

  JSAutoRealm ar(cx, global);

  if (!DefineFunctions(cx, global)) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  if (!ExecuteCode(cx, R"js(
for (let i = 0; i < 10; i++) {
  print(`in main thread, it is ${new Date()}`);
  sleep(1000);
}
  )js")) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  pthread_join(thread1, nullptr);
  pthread_join(thread2, nullptr);

  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(WorkerExample)) {
    return 1;
  }
  return 0;
}
