#include <map>
#include <string>

#include <jsapi.h>
#include <jsfriendapi.h>

#include <js/CompilationAndEvaluation.h>
#include <js/Initialization.h>
#include <js/Modules.h>
#include <js/SourceText.h>

#include "boilerplate.h"

// This examples demonstrates how to compile ES modules in an embedding.
//
// See 'boilerplate.cpp' for the parts of this example that are reused in many
// simple embedding examples.

// Translates source code into a JSObject representing the compiled module. This
// module is not yet linked/instantiated.
static JSObject* CompileExampleModule(JSContext* cx, const char* filename,
                                      const char* code) {
  JS::CompileOptions options(cx);
  options.setFileAndLine(filename, 1);

  JS::SourceText<mozilla::Utf8Unit> source;
  if (!source.init(cx, code, strlen(code), JS::SourceOwnership::Borrowed)) {
    return nullptr;
  }

  // Compile the module source to bytecode.
  //
  // NOTE: This generates a JSObject instead of a JSScript. This contains
  // additional metadata to resolve imports/exports. This object should not be
  // exposed to other JS code or unexpected behaviour may occur.
  return JS::CompileModule(cx, options, source);
}

// Maintain a registry of imported modules. The ResolveHook may be called
// multiple times for the same specifier and we need to return the same compiled
// module.
//
// NOTE: This example assumes only one JSContext/GlobalObject is used, but in
// general the registry needs to be distinct for each GlobalObject.
static std::map<std::u16string, JS::PersistentRootedObject> moduleRegistry;

// Callback for embedding to provide modules for import statements. This example
// hardcodes sources, but an embedding would normally load files here.
static JSObject* ExampleResolveHook(JSContext* cx,
                                    JS::HandleValue modulePrivate,
                                    JS::HandleObject moduleRequest) {
  // Extract module specifier string.
  JS::Rooted<JSString*> specifierString(
      cx, JS::GetModuleRequestSpecifier(cx, moduleRequest));
  if (!specifierString) {
    return nullptr;
  }

  // Convert specifier to a std::u16char for simplicity.
  JS::UniqueTwoByteChars specChars(JS_CopyStringCharsZ(cx, specifierString));
  if (!specChars) {
    return nullptr;
  }
  std::u16string filename(specChars.get());

  // If we already resolved before, return same module.
  auto search = moduleRegistry.find(filename);
  if (search != moduleRegistry.end()) {
    return search->second;
  }

  JS::RootedObject mod(cx);

  if (filename == u"a") {
    mod = CompileExampleModule(cx, "a", "export const C1 = 1;");
    if (!mod) {
      return nullptr;
    }
  }

  if (filename == u"b") {
    mod = CompileExampleModule(cx, "b", "export const C2 = 2;");
    if (!mod) {
      return nullptr;
    }
  }

  // Register result in table.
  if (mod) {
    moduleRegistry.emplace(filename, JS::PersistentRootedObject(cx, mod));
    return mod;
  }

  JS_ReportErrorASCII(cx, "Cannot resolve import specifier");
  return nullptr;
}

// Callback for embedding to implement an asynchronous dynamic import. This must
// do the same thing as the module resolve hook, but also link and evaluate the
// module, and it must always call JS::FinishDynamicModuleImport when done.
static bool ExampleDynamicImportHook(JSContext* cx,
                                     JS::Handle<JS::Value> referencingPrivate,
                                     JS::Handle<JSObject*> moduleRequest,
                                     JS::Handle<JSObject*> promise) {
  JS::Rooted<JSObject*> mod{
      cx, ExampleResolveHook(cx, referencingPrivate, moduleRequest)};
  if (!mod || !JS::ModuleLink(cx, mod)) {
    return JS::FinishDynamicModuleImport(cx, nullptr, referencingPrivate,
                                         moduleRequest, promise);
  }

  JS::Rooted<JS::Value> rval{cx};
  if (!JS::ModuleEvaluate(cx, mod, &rval)) {
    return JS::FinishDynamicModuleImport(cx, nullptr, referencingPrivate,
                                         moduleRequest, promise);
  }
  if (rval.isObject()) {
    JS::Rooted<JSObject*> evaluationPromise{cx, &rval.toObject()};
    return JS::FinishDynamicModuleImport(
      cx, evaluationPromise, referencingPrivate, moduleRequest, promise);
  }
  return JS::FinishDynamicModuleImport(cx, nullptr, referencingPrivate,
                                       moduleRequest, promise);
}

static bool ModuleExample(JSContext* cx) {
  // In order to use dynamic imports, we need a job queue. We can use the
  // default SpiderMonkey job queue for this example, but a more sophisticated
  // embedding would use a custom job queue to schedule its own tasks.
  if (!js::UseInternalJobQueues(cx)) return false;

  // We must instantiate self-hosting *after* setting up job queue.
  if (!JS::InitSelfHostedCode(cx)) return false;

  JS::RootedObject global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    return false;
  }

  JSAutoRealm ar(cx, global);

  // Register a hook in order to provide modules
  JSRuntime* rt = JS_GetRuntime(cx);
  JS::SetModuleResolveHook(rt, ExampleResolveHook);
  JS::SetModuleDynamicImportHook(rt, ExampleDynamicImportHook);

  // Compile the top module.
  static const char top_module_source[] = R"js(
    import {C1} from 'a';
    const {C2} = await import('b');
  )js";
  JS::Rooted<JSObject*> mod{cx,
                            CompileExampleModule(cx, "top", top_module_source)};
  if (!mod) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  // Resolve imports by loading and compiling additional scripts.
  if (!JS::ModuleLink(cx, mod)) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  // Result value, used for top-level await.
  JS::RootedValue rval(cx);

  // Execute the module bytecode.
  if (!JS::ModuleEvaluate(cx, mod, &rval)) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  js::RunJobs(cx);
  if (rval.isObject()) {
    JS::Rooted<JSObject*> evaluationPromise{cx, &rval.toObject()};
    if (!JS::ThrowOnModuleEvaluationFailure(
            cx, evaluationPromise,
            JS::ModuleErrorBehaviour::ThrowModuleErrorsSync)) {
      boilerplate::ReportAndClearException(cx);
      return false;
    }
  }

  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(ModuleExample, /* initSelfHosting = */ false)) {
    return 1;
  }
  return 0;
}
