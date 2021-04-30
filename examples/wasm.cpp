#include <cstdio>

#include <jsapi.h>
#include <js/CompilationAndEvaluation.h>
#include <js/SourceText.h>
#include <js/WasmModule.h>
#include <js/ArrayBuffer.h>

#include "boilerplate.h"

// This example illustrates usage of WebAssembly JS API via embedded
// SpiderMonkey. It does no error handling and simply exits if something
// goes wrong.
//
// See 'boilerplate.cpp' for the parts of this example that are reused in many
// simple embedding examples.
//
// To use the WebAssembly JIT you need to create a context and a global object,
// and  do some setup on both of these. You also need to enter a "realm"
// (environment within one global object) before you can execute code.

/*
hi.wat:
(module
  (import "env" "bar" (func $bar (param i32) (result i32)))
  (func (export "foo") (result i32)
    i32.const 42
    call $bar
  ))
*/
unsigned char hi_wasm[] = {
  0x00, 0x61, 0x73, 0x6d, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0a, 0x02, 0x60,
  0x01, 0x7f, 0x01, 0x7f, 0x60, 0x00, 0x01, 0x7f, 0x02, 0x0b, 0x01, 0x03,
  0x65, 0x6e, 0x76, 0x03, 0x62, 0x61, 0x72, 0x00, 0x00, 0x03, 0x02, 0x01,
  0x01, 0x07, 0x07, 0x01, 0x03, 0x66, 0x6f, 0x6f, 0x00, 0x01, 0x0a, 0x08,
  0x01, 0x06, 0x00, 0x41, 0x2a, 0x10, 0x00, 0x0b
};
unsigned int hi_wasm_len = 56;

static bool BarFunc(JSContext* cx, unsigned argc, JS::Value* vp) {
  JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
  args.rval().setInt32(args[0].toInt32());
  return true;
}

static bool WasmExample(JSContext* cx) {
  JS::RootedObject global(cx, boilerplate::CreateGlobal(cx));
  if (!global) {
    return false;
  }

  JSAutoRealm ar(cx, global);

  // Get WebAssembly.Module and WebAssembly.Instance constructors.
  JS::RootedValue wasm(cx);
  JS::RootedValue wasmModule(cx);
  JS::RootedValue wasmInstance(cx);
  if (!JS_GetProperty(cx, global, "WebAssembly", &wasm)) return false;
  JS::RootedObject wasmObj(cx, &wasm.toObject());
  if (!JS_GetProperty(cx, wasmObj, "Module", &wasmModule)) return false;
  if (!JS_GetProperty(cx, wasmObj, "Instance", &wasmInstance)) return false;


  // Construct Wasm module from bytes.
  JS::RootedObject module_(cx);
  {
    JSObject* arrayBuffer = JS::NewArrayBufferWithUserOwnedContents(cx, hi_wasm_len, hi_wasm);
    if (!arrayBuffer) return false;
    JS::RootedValueArray<1> args(cx);
    args[0].setObject(*arrayBuffer);

    if (!Construct(cx, wasmModule, args, &module_)) return false;
  }

  // Construct Wasm module instance with required imports.
  JS::RootedObject instance_(cx);
  {
    // Build "env" imports object.
    JS::RootedObject envImportObj(cx, JS_NewPlainObject(cx));
    if (!envImportObj) return false;
    if (!JS_DefineFunction(cx, envImportObj, "bar", BarFunc, 1, 0)) return false;
    JS::RootedValue envImport(cx, JS::ObjectValue(*envImportObj));
    // Build imports bag.
    JS::RootedObject imports(cx, JS_NewPlainObject(cx));
    if (!imports) return false;
    if (!JS_SetProperty(cx, imports, "env", envImport)) return false;

    JS::RootedValueArray<2> args(cx);
    args[0].setObject(*module_.get()); // module
    args[1].setObject(*imports.get());// imports

    if (!Construct(cx, wasmInstance, args, &instance_)) return false;
  }

  // Find `foo` method in exports.
  JS::RootedValue exports(cx);
  if (!JS_GetProperty(cx, instance_, "exports", &exports)) return false;
  JS::RootedObject exportsObj(cx, &exports.toObject());
  JS::RootedValue foo(cx);
  if (!JS_GetProperty(cx, exportsObj, "foo", &foo)) return false;

  JS::RootedValue rval(cx);
  if (!Call(cx, JS::UndefinedHandleValue, foo, JS::HandleValueArray::empty(), &rval))
    return false;

  printf("The answer is %d\n", rval.toInt32());
  return true;
}

int main(int argc, const char* argv[]) {
  if (!boilerplate::RunExample(WasmExample)) {
    return 1;
  }
  return 0;
}
