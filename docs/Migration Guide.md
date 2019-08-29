# Migration Guide #

This document describes how to port your code from using one ESR version
of SpiderMonkey to the next ESR version.

## ESR 60 to ESR 68 ##

### Remove requests ###

Previously, a "request" was a global lock on the JS interpreter that was
required in order to use many parts of the JSAPI.
This is no longer necessary, and can simply be deleted.

**Recommendation**: Remove all `JSAutoRequest`, `JS_BeginRequest()`, and
`JS_EndRequest()` from your code.

### Rename compartments to realms ###

In ESR 68, a "realm" is the term for the environment belonging to a
particular global object, so when you enter the "realm" of a global
object you are executing code with that object as the global object.
Unlike with compartments, objects from different realms no longer need
to be "wrapped" to access each other unless they are also
cross-compartment.

JSAutoCompartment is renamed to JSAutoRealm, and CompartmentOptions are
renamed to RealmOptions.
Some APIs that previously operated explicitly on a global object, now
operate on the current realm's global object, but there are currently no
examples of that in this code.

"Compartments" still exist, but they are now more focused on security,
and are used for isolating code in different browser tabs in Firefox,
for example.
Realms are used within the same compartment for code that runs using
different global objects, for example if you wanted to isolate code that
changes `Array.prototype` from your other code.
All code in these examples is run within the same compartment and realm,
so there is not really an example of when objects need to be wrapped.

**Recommendations**:
- Rename `JSAutoCompartment` to `JSAutoRealm`, `JS_EnterCompartment()`
  to `JS::EnterRealm()`, `JS_LeaveCompartment()` to `JS::LeaveRealm()`,
  and `JS::CompartmentOptions` to `JS::RealmOptions`.
- In most cases, rename `JSCompartment` to `JS::Realm`.
  In a few APIs such as weak pointer callbacks where compartments and
  not realms are used, rename it instead to `JS::Compartment`.
- Change APIs such as `JS_GetFunctionPrototype()` which previously
  operated explicitly on a global object, to the new APIs that operate
  implicitly on the global object of the current realm (in this example,
  `JS::GetRealmFunctionPrototype()`.)
- Remove calls to `JS_WrapObject()` where applicable.

### Use UTF8-aware compilation and evaluation ###

In ESR 68, the `JS::Evaluate()` and `JS::Compile()` family of APIs have
been simplified and made UTF8-aware.
Instead of C strings of `char*` and `char16_t*`, they now take an
instance of `JS::SourceText<mozilla::Utf8Unit>` or
`JS::SourceText<char16_t>`, respectively, which in addition to making
the text encoding explicit, also carries information about the ownership
of the source text memory.

**Recommendations:**
- Create an instance of `JS::SourceText` and put your code in it using
  its `init()` method.
  Note that the `init()` method can fail.
- Pass the `JS::SourceText` instance to `JS::Compile()` or
  `JS::Evaluate()` instead of a C string and its length.
- Remove any usage of `JS::CompileOptions::setUTF8()`, which is now made
  obsolete by the type of `JS::SourceText` passed in.
- A few other APIs have been renamed for clarity, such as
  `JS_BufferIsCompilableUnit()` to `JS_UTF8BufferIsCompilableUnit()`.
- If using a source hook to manage in-memory sources,
  `js::SourceHook::load()` now has two out parameters for either UTF8
  source code or wide char source code.
  Use only one of them.
  (None of these examples currently use source hooks.)
- Your code will need to include `<js/SourceText.h>` for
  `JS::SourceText`.

### String encoding ###

`JS_EncodeString()` has been replaced with `JS_EncodeStringToASCII()`
and `JS_EncodeStringToUTF8()`, so that the encoding of the returned
string is always clear.

**Recommendations:**
- Depending on what you were using `JS_EncodeString()` for, replace it
  with either the ASCII or UTF8 version.
  For better performance in cases where you know the string only
  contains ASCII characters, use the ASCII version, otherwise use the
  UTF8 version.
- These functions now return a `JS::UniqueChars` smart pointer, so there
  is no longer any need to `JS_free()` the return value.
- Where possible, change any of your internal APIs where the return
  value of `JS_EncodeString()` was returned, to also return a smart
  pointer.

### Headers ###

There are now more optional headers which should be included separately
in code which uses their functionality.
This is a list of common ones that might be used by embeddings, but in
general if you are missing function definitions when compiling your
code, try checking if you might have to include another header.

- `JS::Compile()`, `JS::Evaluate()`, and similar functions —
  `<js/CompilationAndEvaluation.h>`
- `JS::ContextOptions` — `<js/ContextOptions.h>`
- `JS::WarnUTF8()` and similar functions, and `JS::SetWarningReporter()`
  — `<js/Warnings.h>`

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS_DefineProperty()` for a property with getter and setter function
  objects no longer needs to use `JS_DATA_TO_FUNC_PTR()`.
  There is an appropriate overload to use instead.
- `JS_GetGlobalForObject()` → `JS::GetNonCCWObjectGlobal()`
- `JS_NewArrayBuffer()` → `JS::NewArrayBuffer()`; ditto for
  `JS_NewArrayBufferWithContents()` and `JS_NewExternalArrayBuffer()`.
  In addition, `JS::NewExternalArrayBuffer()` has dropped support for
  the ref callback.
- `JS_ObjectIsFunction()` → `js::IsFunctionObject()`
- `JS_ReportWarningASCII()` → `JS::WarnASCII()`
- `JS_ReportWarningUTF8()` → `JS::WarnUTF8()`
- `JS::AutoIdVector` → `JS::RootedIdVector`, `JS::HandleIdVector`, or
  `JS::MutableHandleIdVector`; ditto for `JS::AutoObjectVector` and
  `JS::AutoValueVector`
- `JS::CallArgs::computeThis()` now takes a `JS::MutableHandleObject`
  out parameter, instead of returning a `JS::Value` which then must be
  converted to an object.
- `JS::CurrentThreadIsHeapCollecting()` →
  `JS::RuntimeHeapIsCollecting()`
- `JS::FormatStackDump()` has removed its input buffer parameter.
- `JS::GCForReason()` → `JS::NonIncrementalGC()`
- `JS::GCPolicy<T>::initial()` → `JS::SafelyInitialized<T>`
