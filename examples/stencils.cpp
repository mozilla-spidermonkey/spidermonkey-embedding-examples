#include <cstdio>
#include <cstdint>
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <map>
#include <mutex>

#include <jsapi.h>
#include <js/CompilationAndEvaluation.h>
#include <js/Conversions.h>
#include <js/Initialization.h>
#include <js/SourceText.h>
#include <js/experimental/JSStencil.h>
#include <js/experimental/CompileScript.h>

#include "boilerplate.h"

// This example illustrates how to use Stencils to optimise performance
// by avoiding repetitive compilations. It shows how to handle
// compilation errors, otherwise it does no error handling and simply exits
// if something goes wrong.
//
// To reuse Stencils in multiple threads, you must create a JS::FrontendContext
// in each thread that compiles Javascript.

// Helper output
static std::ostream &labeled_cout() {
  return std::cout << "Thread: " << std::hex << std::this_thread::get_id()
                   << " ";
}

// Cache of compiled scripts
class JSCache {
 public:
  ~JSCache() {
    labeled_cout() << "Destructing cache with " << _cache.size()
                   << " scripts\n";
  }

  RefPtr<JS::Stencil> find(std::string const &key) {
    std::lock_guard<std::mutex> lk(_mx);
    auto it = _cache.find(key);
    return it == _cache.end() ? nullptr : it->second;
  }

  void insert(std::string const &key, RefPtr<JS::Stencil> const &val) {
    std::lock_guard<std::mutex> lk(_mx);
    _cache.insert(std::pair{key, val});
  }

 private:
  std::mutex _mx;
  std::map<std::string, RefPtr<JS::Stencil>> _cache;
};

// Script compiler and executer
class Job {
 public:
  Job(JSCache &cache);
  ~Job() {
    if (_fc) JS::DestroyFrontendContext(_fc);
  };

  void execute_script(JSContext *cx, std::string const &script,
                      const char *filename, unsigned long linenumber);

 private:
  RefPtr<JS::Stencil> compile_script(JSContext *cx, std::string const &script,
                                     const char *filename,
                                     unsigned long linenumber);

 private:
  JSCache &_cache;
  JS::FrontendContext *_fc;
};

Job::Job(JSCache &cache) : _cache(cache) {
  static constexpr size_t kCompileStackQuota = 128 * sizeof(size_t) * 1024;
  _fc = JS::NewFrontendContext();
  if (_fc) JS::SetNativeStackQuota(_fc, kCompileStackQuota);
}

void Job::execute_script(JSContext *cx, std::string const &script,
                         const char *filename, unsigned long linenumber) {
  JS::RootedScript rscript(cx);
  RefPtr<JS::Stencil> stencil(_cache.find(script));

  if (stencil == nullptr) {
    labeled_cout() << "Compiling script\n";
    stencil = compile_script(cx, script, filename, linenumber);
    if (stencil != nullptr) _cache.insert(script, stencil);
  } else {
    labeled_cout() << "Taking script from the cache\n";
  }

  if (stencil == nullptr) {
    boilerplate::ReportAndClearException(cx);
    return;
  }

  JS::InstantiateOptions instantiateOptions;
  rscript = JS::InstantiateGlobalStencil(cx, instantiateOptions, stencil);
  if (!rscript) {
    boilerplate::ReportAndClearException(cx);
    return;
  }

  JS::RootedValue val(cx);
  if (!JS_ExecuteScript(cx, rscript, &val)) {
    boilerplate::ReportAndClearException(cx);
    return;
  }
}

RefPtr<JS::Stencil> Job::compile_script(JSContext *cx,
                                        std::string const &script,
                                        const char *filename,
                                        unsigned long linenumber) {
  JS::SourceText<mozilla::Utf8Unit> source;
  if (!_fc) return nullptr;

  JS::CompileOptions opts(cx);
  opts.setFileAndLine(filename, linenumber);
  opts.setNonSyntacticScope(true);

  if (!source.init(_fc, script.c_str(), script.size(),
                   JS::SourceOwnership::Borrowed)) {
    labeled_cout() << "Error initializing JS source\n";
    ConvertFrontendErrorsToRuntimeErrors(cx, _fc, opts);
    JS::ClearFrontendErrors(_fc);
    return nullptr;
  }

  JS::CompilationStorage compileStorage;
  RefPtr<JS::Stencil> st =
      JS::CompileGlobalScriptToStencil(_fc, opts, source, compileStorage);

  if (st == nullptr) {
    labeled_cout()
        << "Error compiling script, presumably due to a syntax error.\n";
    // Let boilerplate::ReportAndClearException to report the error.
    ConvertFrontendErrorsToRuntimeErrors(cx, _fc, opts);
    JS::ClearFrontendErrors(_fc);
  }

  return st;
}

////////////////////////////////////////////////////////////////////////////////
// Code to illustrate how to use stencils and the cache

static bool Print(JSContext *cx, unsigned argc, JS::Value *vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

  JS::Rooted<JS::Value> arg(cx, args.get(0));
  JS::Rooted<JSString *> str(cx, JS::ToString(cx, arg));
  if (!str) return false;

  JS::UniqueChars chars = JS_EncodeStringToUTF8(cx, str);
  labeled_cout() << chars.get() << "\n";

  args.rval().setUndefined();
  return true;
}

bool DefineFunctions(JSContext *cx, JS::Handle<JSObject *> global) {
  return JS_DefineFunction(cx, global, "print", &Print, 0, 0);
}

static void ExecuteExamples(JSContext *cx, Job &job) {
  // a few JS code snippets, one with a syntax error
  static int jsline = __LINE__;
  static char const *js1 = R"js(print(`JS log one: ${new Date()}`);)js";
  static char const *js2 = R"js(await print(`JS log two: ${new Date()}`);)js";
  static char const *js3 = R"js(print(`JS log three: ${new Date()}`);)js";

  static char const *scripts[] = {js1, js2, js3};

  JS::Rooted<JSObject *> global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    labeled_cout() << "Failed during boilerplate::CreateGlobal\n";
    return;
  }

  JSAutoRealm ar(cx, global);

  if (!DefineFunctions(cx, global)) {
    boilerplate::ReportAndClearException(cx);
    return;
  }

  unsigned long linenumber = jsline;  // for error reports
  for (auto code : scripts) {
    job.execute_script(cx, code, __FILE__, ++linenumber);
  }
}

static void ThreadFunction(JSRuntime *parentRuntime, JSCache *cache) {
  JSContext *cx(JS_NewContext(8L * 1024L * 1024L, parentRuntime));
  Job job(*cache);

  labeled_cout() << "Child thread started\n";

  if (!JS::InitSelfHostedCode(cx)) {
    labeled_cout() << "Failed during JS::InitSelfHostedCode\n";
    JS_DestroyContext(cx);
    return;
  }

  for (int i = 0; i < 2; ++i) ExecuteExamples(cx, job);

  JS_DestroyContext(cx);
}

static bool StencilExample(JSContext *cx) {
  JSCache cache;

  labeled_cout() << "Main thread started\n";

  std::thread thread(ThreadFunction, JS_GetRuntime(cx), &cache);

  Job job(cache);
  for (int i = 0; i < 2; ++i) ExecuteExamples(cx, job);

  thread.join();

  return true;
}

int main(int argc, const char *argv[]) {
  if (!boilerplate::RunExample(StencilExample)) {
    return 1;
  }
  return 0;
}
