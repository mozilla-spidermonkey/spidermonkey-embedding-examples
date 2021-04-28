# Migration Guide #

This document describes how to port your code from using one ESR version
of SpiderMonkey to the next ESR version.

## ESR 78 to ESR 90 ##

- JS::ExceptionStack now requires including js/Exception.h (Bug 1626100)
- JS_New replaced with JS::Construct. (Bug 1491055)
- SetModuleResolveHook changed signature. (Bug 1668330)

## ESR 68 to ESR 78 ##

### Requirements ###

A C++ compiler that can support C++17 is now required.

### Flat strings ###

There is no longer any distinction made between flat strings and linear strings.

**Recommendation:** For everything that was done with `JSFlatString`, simply use `JSLinearString` instead.
`js::FlatStringToLinearString()` is no longer necessary and can be removed altogether.

### JSClass ###

There is no longer any distinction made between the public `JSClass` API and the semi-private `js::Class` API.
Both are `JSClass` now.

**Recommendation:** Embeddings most probably did not need to use `js::Class` anyway, but if you did, you can convert it to `JSClass`.
The fields of these two structs are mostly the same.
The function to convert between the two, `js::Jsvalify()`, can be removed altogether.

### Error reports ###

It is no longer possible to access the `flags` member of `JSErrorReport`, and the flags constants have been removed.

**Recommendation:** Instead of checking for `JSREPORT_ERROR` and `JSREPORT_WARNING`, use the `JSErrorReport::isWarning()` method. There is no equivalent for `JSREPORT_EXCEPTION` and `JSREPORT_STRICT`.

Additionally, instead of `JS_ErrorFromException()` and `JSErrorReport`, consider using `JS::StealPendingException()` and `JS::ErrorReportBuilder`.

### Security changes ###

`JS::PromiseRejectionTrackerCallback` callbacks now take an extra `bool mutedErrors` parameter.
This is to prevent details from being leaked to untrusted cross-origin scripts via error messages.

The `uneval()` function and the `toSource()` methods are now disabled by default, but they can be re-enabled per realm using `JS::RealmCreationOptions::setToSourceEnabled()`.

**Recommendation:** If your embedding is only running trusted code, i.e. you are not setting the `mutedErrors` flag on any `JS::CompileOptions`, then you can ignore this argument in your callback.
If you are using the muted errors feature, then make sure that your callback doesn't perform any observable actions if this argument is `true`.

If you were using `toSource()` or `uneval()` you can keep them enabled in the `JS::Realm` where they are needed, but it's probably best to find an alternative for this functionality.

### Debugger API ###

The Debugger API has removed the `Debugger.enabled` property.

**Recommendation:** Instead of setting the `enabled` property to `false`, use the `removeAllDebuggees()` method.
Instead of setting the property to `true`, add new debuggee objects.

### Defaults ###

There are some things that were optional in ESR 68 that are now the default and can be omitted:

- `JSGC_DYNAMIC_MARK_SLICE`
- `JSGC_DYNAMIC_HEAP_GROWTH`
- `JS::RealmCreationOptions::setBigIntEnabled()`
- `JS::RealmCreationOptions::setFieldsEnabled()`

### Headers ###

There are now more optional headers which should be included separately
in code which uses their functionality.
This is a list of common ones that might be used by embeddings, but in
general if you are missing function definitions when compiling your
code, try checking if you might have to include another header.

- `JS::NewArrayObject()`, `JS::GetArrayLength()`, and similar functions —
  `<js/Array.h>`
- `JS::RootedValueArray`, `JS::HandleValueArray`, and similar types — `<js/ValueArray.h>`
- `<js/ComparisonOperators.h>` is now needed for certain types' `operator==()` and `operator!=()`, although you will probably not need to include this directly.

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS_NewArrayObject()` → `JS::NewArrayObject()`
- `JS::AutoValueArray` → `JS::RootedValueArray`
- `INTERNED_STRING_TO_JSID()` → `JS::PropertyKey::fromPinnedString()`
- `JS_IsArrayObject()` → `JS::IsArrayObject()`
- `JS_GetArrayLength()` → `JS::GetArrayLength()`
- `JSID_TO_FLAT_STRING()` → `JSID_TO_LINEAR_STRING()`
- `JS_ASSERT_STRING_IS_FLAT()` → `JS_ASSERT_STRING_IS_LINEAR()`
- `JS_FORGET_STRING_FLATNESS()` → `JS_FORGET_STRING_LINEARNESS()`
- `JS_StringIsFlat()` → `JS_StringIsLinear()`
- `js::gc::detail::GetGCThingZone()` → `JS::GetGCThingZone()`
- `JSGC_SLICE_TIME_BUDGET` → `JSGC_SLICE_TIME_BUDGET_MS`
- `JSGCCallback` now takes an extra `JS::GCReason` parameter indicating why the garbage collection was initiated.
- `js::GetCodeCoverageSummary()` returns `JS::UniqueChars` instead of `char*`.
- `JSGC_MAX_MALLOC_BYTES` no longer did anything, so was removed.

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

### Hash tables ###

It is no longer necessary to initialize `JS::GCHashMap<>` with `init()`.

**Recommendation:** Simply remove the call to `init()`, or if you were
using it to reserve space in the hash map while initializing it, replace
it with a call to `reserve()`.

### Principals ###

Functions having to do with stack frames, such as
`JS::GetSavedFrameSource()` and others, now take a `JSPrincipals*`
pointer as an argument.
This is a trust token that indicates the privileges of the current
caller.
The stack frame APIs use this to appropriately hide system stack frames
from less trusted code.

**Recommendations:**
- If you are using `JSPrincipals` in your code, then pass the
  appropriate one to these functions.
- If not, you can find the current context's principal with
  `cx->realm()->principals()`.
  However, if you are not using `JS_SetSecurityCallbacks()` then you can
  probably just pass `nullptr` for the `JSPrincipals` argument.

### C-allocated memory ###

Previously when associating C-allocated memory with a JS object, the API
to use was `JS_updateMallocCounter()`.
This inexact API is now gone.
Instead, you can tell the JS engine exactly how much memory is
associated with a particular JS object with `JS::AddAssociatedMemory()`.

Note that each call to `JS::AddAssociatedMemory()` must be exactly
matched by a call to `JS::RemoveAssociatedMemory()`.

**Recommendation:** You may need to restructure your code if you were
using `JS_updateMallocCounter()`.
It may be useful to call `JS::AddAssociatedMemory()` in your `JSClass`'s
constructor, and `JS::RemoveAssociatedMemory()` in the finalizer.

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
- `JS_InitStandardClasses()` → `JS::InitRealmStandardClasses()`
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
