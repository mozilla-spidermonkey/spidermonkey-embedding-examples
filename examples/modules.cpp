#include <map>
#include <string>

#include <jsapi.h>

#include <js/CompilationAndEvaluation.h>
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

  // Register result in table.
  if (mod) {
    moduleRegistry.emplace(filename, JS::PersistentRootedObject(cx, mod));
    return mod;
  }

  JS_ReportErrorASCII(cx, "Cannot resolve import specifier");
  return nullptr;
}

static bool ModuleExample(JSContext* cx) {
  JS::RootedObject global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    return false;
  }

  JSAutoRealm ar(cx, global);

  // Register a hook in order to provide modules
  JS::SetModuleResolveHook(JS_GetRuntime(cx), ExampleResolveHook);

  // Compile the top module.
  JS::RootedObject mod(
      cx, CompileExampleModule(cx, "top", "import {C1} from 'a';"));
  if (!mod) {
    boilerplate::ReportAndClearException(cx);
    return false;
  }

  // Resolve imports by loading and compiling additional scripts.
  if (!JS::ModuleInstantiate(cx, mod)) {
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

  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(ModuleExample)) {
    return 1;
  }
  return 0;
}
