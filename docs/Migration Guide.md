# Migration Guide #

This document describes how to port your code from using one ESR version
of SpiderMonkey to the next ESR version.

## ESR 102 to ESR 115 ##

### More Versatile `JS_InitClass` ###

In previous versions of SpiderMonkey, the name of the constructor
function added by `JS_InitClass` was always the name of the prototype
object's `JSClass`.
This limitation caused the instances and the prototype to always share
the same `JSClass` and `JSClassOps` specification.
If the prototype should behave differently than the instances, custom
checks were needed.
Now `JS_InitClass` accepts a `const char *` parameter for naming the
constructor function, so a different `JSClass *` can be used for the
prototype object.
If `nullptr` is passed, the prototype object will be a plain JS object.
See [this patch from bug 1808171](https://hg.mozilla.org/releases/mozilla-esr115/rev/15e1e69037e370750f704d97d631e23ef32f3812).

### Bound Function Objects ###

Bound function objects (callable functions created with
`Function.prototype.bind()`) are no longer `JSFunction` objects.
`JS::GetBuiltinClass()` will not return `js::ESClass::Function` for such
objects.

Unless you are using `JS::GetBuiltinClass()` to determine whether an
object is callable, you will most likely not have to migrate anything.
Consider using `JS::IsCallable()`.

If you need to check whether an object is specifically a bound function
object, use `JS_ObjectIsBoundFunction()`.

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS::ModuleInstantiate` → `JS::ModuleLink`
- `mozilla::Tuple` → `std::tuple` (`<mozilla/Tuple.h>` is removed, use
  the standard C++ header `<tuple>` instead)
- `mozilla::IsFinite()` → `std::isfinite()`
- `mozilla::IsNaN()` → `std::isnan()`
- Script filenames are now always encoded as UTF-8, see
  [this patch from bug 1492090](https://hg.mozilla.org/releases/mozilla-esr115/rev/416af93c3205460856a2cae7bee084a656ee2ee9)

## ESR 91 to ESR 102 ##

### Object private pointers ###

In previous versions of SpiderMonkey, it was common to associate C++
structs with JS objects by using `JS_SetPrivate()` and `JS_GetPrivate()`
on the JS object. These APIs have been removed.

Instead, stuff the C++ struct pointer into a `JS::PrivateValue` and put
it in one of the object's reserved slots with `JS::SetReservedSlot()`.

**Recommendation:** To do this, you will have to change a few things.
First, the `JSCLASS_HAS_PRIVATE` flag no longer exists, so you need to
remove it and instead give the class an additional reserved slot in its
`JSClass` definition.
For example:

```c++
// old
static constexpr JSClass klass = {
    "MyClass",
    JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(2) | JSCLASS_BACKGROUND_FINALIZE,
    &class_ops,
};

// new
static constexpr JSClass klass = {
    "MyClass",
    JSCLASS_HAS_RESERVED_SLOTS(3) | JSCLASS_BACKGROUND_FINALIZE,
    &class_ops,
};
```

Then, replace `JS_SetPrivate()` with `JS::SetReservedSlot()` and
`JS::PrivateValue`.
Note that to set a null pointer, you should store `JS::UndefinedValue()`
in the reserved slot, not `JS::PrivateValue(nullptr)`.

```c++
// old
JS_SetPrivate(obj1, ptr);
JS_SetPrivate(obj2, nullptr);

// new
JS::SetReservedSlot(obj1, POINTER, JS::PrivateValue(ptr));
JS::SetReservedSlot(obj2, POINTER, JS::UndefinedValue());
// (POINTER is an enum value indicating which reserved slot stores the
// private data. For example, it could be 0. You define it yourself.)
```

Finally, replace `JS_GetPrivate()` with
`JS::GetMaybePtrFromReservedSlot()`, which takes a template parameter
indicating the type of the pointer.

```c++
// old
MyData* ptr = static_cast<MyData*>(JS_GetPrivate(obj1));

// new
MyData* ptr = JS::GetMaybePtrFromReservedSlot<MyData>(obj1, POINTER);
```

These changes can all be done before the migration, except that
SpiderMonkey 91 does not have `JS::GetMaybePtrFromReservedSlot()`.
To work around this, you can open-code it and later remove it after the
migration:

```c++
template <typename T>
inline T* MyGetMaybePtrFromReservedSlot(JSObject* obj, size_t slot) {
    JS::Value v = JS::GetReservedSlot(obj, slot);
    return v.isUndefined() ? nullptr : static_cast<T*>(v.toPrivate());
}
```

Additionally, if you have any `JS::Heap<>` pointers to GC things as
members of your private C++ structs, this migration might be a good time
to consider moving those GC things into their own reserved slots on the
object, and just keep C++ data in the C++ struct.
This technique might not be right for every codebase, but it's worth
considering.

### Property keys ###

As in the previous migration from 78 to 91, more JSID macros have now
been replaced by methods of `JS::PropertyKey`.

**Recommendation:** Before the migration, replace `JSID_IS_VOID` (or an
equality comparison to `JSID_VOID`) with `id.isVoid()`.

After the migration, replace the following:
- `JSID_VOID` → `JS::PropertyKey::Void()`
- `SYMBOL_TO_JSID()` → `JS::PropertyKey::Symbol()`
- `JSID_BITS(id)` → `id.asRawBits()`
- `JSID_TO_LINEAR_STRING(id)` → `id.toLinearString()`

### Headers ###

There are now more headers which should be included separately in code
which uses their functionality.

This is a list of common ones that might be used by embeddings, but in
general if you are missing function definitions when compiling your
code, try checking if you might have to include another header.

- `JS::Call`, `JS::Construct`, and related — `<js/CallAndConstruct.h>`
- `JS_DefineDebuggerObject()` — `<js/Debug.h>`
- `JS::CurrentGlobalOrNull()` — `<js/GlobalObject.h>`
- `JS_DefineProperty()`, `JS_DefineFunction()`, `JS_Enumerate()`,
  `JS_GetElement()`, `JS_GetProperty()`, `JS_HasProperty()`,
  `JS_SetProperty()`, and many other functions related to accessing
  properties or array elements — `<js/PropertyAndElement.h>`
- `JS::GetScriptPrivate()` and `JS::SetScriptPrivate()` —
  `<js/ScriptPrivate.h>`
- `JS::BuildStackString()` and other functions related to the call stack
  — `<js/Stack.h>`

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS::PropertyKey::fromNonIntAtom()` → `JS::PropertyKey::NonIntAtom()`
- `JS::SafelyInitialized<T>()` → `JS::SafelyInitialized<T>::create()`
- `JSFreeOp` → `JS::GCContext` (in particular, in finalize operations)
- `JS_UpdateWeakPointerAfterGC()` and weak-pointer update callbacks now
  take an additional `JSTracer*` argument.

The following things have been removed.
In several cases, you can make the necessary changes before the
migration already.

- `JS_AtomizeAndPinJSString()` has been removed, there is no direct
  replacement. There are several `JS_AtomizeAndPin...` APIs that might
  work instead.
- `JS::CloneAndExecuteScript()` has been removed.
  Usually, if the compilation cost isn't significant, `JS::Evaluate()`,
  or `JS::Compile()` followed by `JS_ExecuteScript()`, should be
  sufficient.
  If the reason to use `JS::CloneAndExecuteScript()` was because the
  compilation cost was problematic, then instead use
  `JS::CompileGlobalScriptToStencil()`, and get a `JSScript` in the new
  realm with `JS::InstantiateGlobalStencil()`.
- `JSClassOps::hasInstance` is removed.
  Instead of overriding the `instanceof` behaviour in `JSClassOps`, you
  should define a `[Symbol.hasInstance]` property on the prototype
  object that implements the overridden behaviour.

In addition, `JS::Rooted` had a convenience constructor that would
forward its `JSContext*` argument to the template type `T` if there was
a constructor with the signature `T(JSContext*)`.
This constructor has been removed, so you may need to duplicate a
`JSContext*` argument, e.g.:
```c++
JS::Rooted<JS::IdVector> ids(cx, cx);
```

## ESR 78 to ESR 91 ##

### Object construction ###

In ESR 78 there were two APIs that construct objects as if calling `new
Constructor(...args)`: `JS_New()` and `JS::Construct()`.
`JS_New()` was redundant and has been removed.
(Bug [1491055](https://bugzilla.mozilla.org/show_bug.cgi?id=1491055))

**Recommendation:** Replace all uses of `JS_New()` with the
four-argument version of `JS::Construct()` before doing the migration.
Note that `JS::Construct()` takes the constructor as a `JS::HandleValue`
instead of `JS::HandleObject`, and returns a boolean indicating success
or error.

```c++
// old
JS::RootedObject myConstructor(cx, &myConstructorValue.toObject());
JS::RootedObject myObject(cx, JS_New(cx, myConstructor, args));
if (!myObject) return false;

// new
JS::RootedObject myObject(cx);
if (!JS::Construct(cx, myConstructorValue, args, &myObject)) return false;
```

### Property keys ###

The `jsid` type was already an alias of `JS::PropertyKey`.
Some of the old `JSID_IS_...` and `JSID_TO_...` macro-like APIs still
exist in ESR 91, although there now exist methods of `JS::PropertyKey`
which do the same things.
Others have been removed.
(Bug [1633145](https://bugzilla.mozilla.org/show_bug.cgi?id=1633145))

**Recommendation:** Replace the following before the migration:
- `JSID_IS_STRING(id)` → `id.isString()`
- `JSID_TO_STRING(id)` → `id.toString()`
- `JSID_IS_INT(id)` → `id.isInt()`
- `JSID_TO_INT(id)` → `id.toInt()`
- `JSID_IS_SYMBOL(id)` → `id.isSymbol()`
- `JSID_TO_SYMBOL(id)` → `id.toSymbol()`
- `JSID_IS_GCTHING(id)` → `id.isGCThing()`
- `JSID_TO_GCTHING(id)` → `id.toGCCellPtr()`
- `INTERNED_STRING_TO_JSID(cx, str)` → `JS::PropertyKey::fromPinnedString(str)`
- `NON_INTEGER_ATOM_TO_JSID(str)` → `JS::PropertyKey::fromNonIntAtom(str)`

After the migration, additionally replace `JSID_IS_VOID(id)` → `id.isVoid()`.

`JSID_IS_ATOM()` and `JSID_TO_ATOM()` can be replaced by `isAtom()` and
`toAtom()` respectively, but consider if it might be better to rewrite
those in terms of `isString()` and `toString()`.

### Module resolve hooks ###

Module resolve hooks have changed signature.
Insead of a specifier string, resolve hooks recieve a module request
object, which contains the specifier and in the future potentially other
information as well.
(Bug [1668330](https://bugzilla.mozilla.org/show_bug.cgi?id=1668330))

**Recommendation:** Unless you are using `JS::SetModuleResolveHook()`,
then you don't have to do anything.
If you are, then adapt the signature of your module resolve hook to take
`JS::HandleObject` as its last parameter rather than `JS::HandleString`.
To get the specifier, if you were using it, use
`JS::GetModuleRequestSpecifier(cx, moduleRequestObject)`.

### Top-level await changes ###

There have been some changes to the module evaluation and dynamic import
APIs to accommodate top-level await.
(Bug [1519100](https://bugzilla.mozilla.org/show_bug.cgi?id=1519100))

**Recommendation:** Unless you are using modules in your codebase, no
changes are necessary.
Additionally, the changes are minimal, limited to a few API signatures,
unless you decide to enable top-level await in your codebase.

First, `JS::ModuleEvaluate()` has gained a `JS::MutableHandleValue` out
parameter, which must be passed in, but can otherwise be ignored.
This out parameter is for the Promise that would be returned from a
top-level await operation.

Then, replace any calls to `JS::FinishDynamicModuleImport()` with
`JS::FinishDynamicModuleImport_NoTLA()`.
The latter function now requires a status, either
`JS::DynamicImportStatus::Ok` or `JS::DynamicImportStatus::Failed`.
Additionally, it gets passed the module request object instead of the
module specifier string.

<!-- TODO: Add information on how to enable top-level await. -->

### Script compile options ###

`JS::CompileOptions` now has some different responsibilities regarding
JS scripts.
Setting script private values has moved to a separate API.
Conversely, it's now required to set a flag on `JS::CompileOptions` when
using the scope chain parameter of `JS::Evaluate()` or
`JS_ExecuteScript()`.
(Bug [1702278](https://bugzilla.mozilla.org/show_bug.cgi?id=1702278))

**Recommendations:** First, if you were using scope chains, make sure to
set the appropriate flag on your compile options using
`JS::CompileOptions::setNonSyntacticScope()`.

Then, if you were using `JS::CompileOptions::setPrivateValue()` (likely,
if you were using dynamic module imports; otherwise, probably not),
replace it with
`JS::SetScriptPrivate()`.
This may require splitting a `JS::Evaluate()` call into two separate
steps of `JS::Compile()` and `JS_ExecuteScript()`.

```c++
// old
JS::CompileOptions options(cx);
options.setPrivateValue(v);
if (!JS::Evaluate(cx, options, code, &retval)) return false;

// new
JS::CompileOptions options(cx);
JS::RootedScript script(cx, JS::Compile(cx, options, code));
if (!script) return false;
JS::SetScriptPrivate(script, v);
if (!JS_EvaluateScript(cx, script, &retval)) return false;
```

### Maybe-based property descriptor API ###

`JS_GetPropertyDescriptor()` and similar functions previously used the
confusingly-named `JS::PropertyDescriptor::object()` method on the
returned property descriptor to indicate whether the property was found
or not.
Now these functions use `mozilla::Maybe` in their signatures, and the
`object()` method is gone.
(Bug [1706404](https://bugzilla.mozilla.org/show_bug.cgi?id=1706404))

**Recommendation:** This may require some restructuring of your code.
Instead of using a `JS::Rooted<JS::PropertyDescriptor>` to hold the out
parameter, use `JS::Rooted<mozilla::Maybe<JS::PropertyDescriptor>>`.
On the returned value, check `isSome()` instead of `object()` to see if
a property descriptor exists, and if it does, use `->` instead of `.` to
access its methods.

### Initialization ###

Since SpiderMonkey 31 it's been required to call `JS_Init()` at the
start of the program and `JS_ShutDown()` at the end.
If you haven't been doing this in your code, now is the time to really
do it, because it will otherwise trip a debug assertion.

### Headers ###

There are now more headers which should be included separately in code
which uses their functionality.
In particular, several things from `<jsfriendapi.h>` have moved into
separate headers in a `js/friend/` subdirectory.

This is a list of common ones that might be used by embeddings, but in
general if you are missing function definitions when compiling your
code, try checking if you might have to include another header.

- `JS::ExceptionStack` — `<js/Exception.h>`
  (Bug [1626100](https://bugzilla.mozilla.org/show_bug.cgi?id=1626100))
- `js::GetErrorMessage()` — `<js/friend/ErrorMessages.h>`
  (Bug [1654927](https://bugzilla.mozilla.org/show_bug.cgi?id=1654927))
- `js::DumpValue()`, `js::DumpString()`, and similar functions —
  `<js/friend/DumpFunctions.h>`
  (Bug [1656411](https://bugzilla.mozilla.org/show_bug.cgi?id=1656411))
- `JS::GetPrivate()`, `JS::SetPrivate()`, `JS::GetClass()`,
  `JS::GetReservedSlot()`, `JS::SetReservedSlot()`, and other `Object`
  functions — `<js/Object.h>`
  (Bug [1663365](https://bugzilla.mozilla.org/show_bug.cgi?id=1663365))
- `JS::StringHasLatin1Chars()` and other `String` functions —
  `<js/String.h>`
  (Bug [1663365](https://bugzilla.mozilla.org/show_bug.cgi?id=1663365))
- Various `ArrayBuffer` and typed-array methods —
  `<js/experimental/TypedData.h>`
  (Bug [1656411](https://bugzilla.mozilla.org/show_bug.cgi?id=1656411))
- `JS_ReportOutOfMemory()` — `<js/ErrorReport.h>`
- `JSAutoRealm` — `<js/Realm.h>`
- `JS_GetContextPRivate()` — `<js/Context.h>`

Additionally, `<js/RequiredDefines.h>` is no longer needed.
Usually it would automatically have been included by means of the
pkg-config file, but if you were manually including it, you can remove
it.

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS_GetPrivate()` → `JS::GetPrivate()`
- `JS_SetPrivate()` → `JS::SetPrivate()`
- `JS_GetClass()` → `JS::GetClass()`
- `JS_GetReservedSlot()` → `JS::GetReservedSlot()`
- `JS_SetReservedSlot()` → `JS::SetReservedSlot()`
- `JS_StringHasLatin1Chars()` → `JS::StringHasLatin1Chars()`
- `js::GetLinearStringLength()` → `JS::GetLinearStringLength()`
- `js::LinearStringHasLatin1Chars()` →
  `JS::LinearStringHasLatin1Chars()`
- `js::GetLatin1LinearStringChars()` →
  `JS::GetLatin1LinearStringChars()`
- `js::GetTwoByteLinearStringChars()` →
  `JS::GetTwoByteLinearStringChars()`
- `js::GetUint8ArrayLengthAndData()` and friends now use `size_t` for
  length instead of `uint32_t`
  (Bug [1674777](https://bugzilla.mozilla.org/show_bug.cgi?id=1674777))
- `JS::PrintError()` no longer takes `cx` argument
  (Bug [1711878](https://bugzilla.mozilla.org/show_bug.cgi?id=1711878))
- `JSPROP_SETTER` and `JSPROP_GETTER` are no longer needed
  (Bug [1713083](https://bugzilla.mozilla.org/show_bug.cgi?id=1713083))
- The `JSGCMode` enum and `JSGC_MODE` setting have been replaced by two
  individual boolean settings, `JSGC_INCREMENTAL_GC_ENABLED` and
  `JSGC_PER_ZONE_GC_ENABLED`
  (Bug [1686249](https://bugzilla.mozilla.org/show_bug.cgi?id=1686249))
- The `JSGCInvocationKind` enum has been replaced by `JS::GCOptions`.
  (Bug [1709849](https://bugzilla.mozilla.org/show_bug.cgi?id=1709849))

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

## ESR 10 to 17 ##

At this time the JSAPI was still being documented in MDN.
Viewing the pages on archive.org may give more details.
[This](https://web.archive.org/web/20200424132803/https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference)
is the last available snapshot, but older snapshots may be useful as
well.

### Headers ###

The write barrier API (`JS_RegisterReference()`, `JS_ModifyReference()`,
etc.) has moved from `jsapi.h` to `jsfriendapi.h`.

### C++ ###

At this point in SpiderMonkey's history, its headers could be compiled
as either C or C++.
There are a number of newer APIs behind an `#ifdef __cplusplus`, such as
`JS::Value` and the Handle APIs.

**Recommendation:**
If your codebase is in C, migrate it to C++.
That will have to happen eventually anyway, as the C API gets removed in
a future version.
Migrating now will allow any new code that's being written to opt-in to
future-proof APIs such as `JS::Value`.

### Types ###

Mozilla's custom integer types (`uint8`, `int32`, `JSUint64`, etc.) are
deprecated, although still present in ESR 17.
The standard types from `<stdint.h>` are preferred.

Other types have been removed in favour of plain C types:
- `intN` → `int`
- `jsdouble` → `double`
- `jsint` → `int`
- `jsrefcount` → `unsigned`
- `jsuint` → `unsigned`
- `jsuword` → `uintptr_t`
- `jsword` → `intptr_t`
- `uintN` → `unsigned`

**Recommendation:**
Replace all occurrences of these types.

### Access from other threads ###

The `JS_THREADSAFE` build option is permanently turned on, meaning that
JS APIs cannot be accessed from arbitrary threads.
If your embedding relied on this, the code will probably need some
refactoring.

APIs having to do with multi-threaded JS runtimes have all been removed:
- `JS_ClearContextThread()`
- `JS_GetContextThread()`
- `JS_Lock()`
- `JS_SetContextThread()`
- `JS_Unlock()`

### JSClass ###

In this version there were several changes in the `JSClass` struct and
its associated operations.

First of all, some of the members were reordered.
Hopefully any affected code would fail to compile with the new ESR, but
it's probably good to check all your `JSClass` definitions anyway.

The `JSClass.xdrObject` member was removed.
It's likely that embedders were already not using JSXDR and were already
setting this member to null.
If you used this API, you may have to look for a custom solution.

Previously, a `JSClass` could provide either a `JSMarkOp` or `JSTraceOp`
function pointer as the `JSClass.mark` member.
The `JSCLASS_MARK_IS_TRACE` flag was used to indicate which one it was,
and therefore how the function pointer would be called.
The member has been renamed to `JSClass.trace`, and `JSMarkOp` has been
removed, as well as `JSCLASS_MARK_IS_TRACE`.

If you had any `JSClass` with a non-null `mark` member that _didn't_ use
`JSCLASS_MARK_IS_TRACE`, you must rewrite the mark operation as a trace
operation.
In most cases this should consist of a pretty straightforward
replacement of `JS_MarkGCThing()` with `JS_CALL_TRACER()` and updating
the function signature.
An example is
[here](https://bugzilla.mozilla.org/attachment.cgi?id=516449&action=diff#a/js/src/shell/js.cpp_sec2).

Several other `JSClass` operations, such as `JSPropertyOp`, changed
their signatures to take Handles instead of pointers.
Handles are part of the C++ API, although there are some alternate
[typedefs](https://searchfox.org/mozilla-esr17/source/js/src/jsapi.h#1650)
for use in the C API.

You will need to update the signatures of the class operations to match
the new definitions.
The definition of `JSPropertyOp`, for example, changed as follows:

```c++
// old
typedef JSBool (*JSPropertyOp)(JSContext*, JSObject*, jsid, jsval*);
// new
typedef JSBool (*JSPropertyOp)(JSContext*, JSHandleObject, JSHandleId, JSMutableHandleValue);
```

If you are using C++ (recommended), you may need to use the Handle's
`get()` or `address()` methods if you need access to the underlying
GC-rooted thing, and `set()` if you need to set the value in a mutable
Handle.
For embeddings that still use C, use the `._` member of the ersatz
Handle typedef or just cast the Handle to a pointer to the underlying
GC thing, which should share the same memory layout (e.g. `(JSObject**)`
for `JSHandleObject`).

`JSFinalizeOp`'s `JSContext*` parameter was replaced with a `JSFreeOp*`
parameter.
This is normally not used by embeddings.

There are some other minor removals listed below in the "Various API
changes" section.

**Recommendations:**
- Double-check your `JSClass` definitions to make sure the members
  match the new order.
- Remove any usage of `JSCLASS_MARK_IS_TRACE`.
- If you have any `JSMarkOp` functions, port them to match the signature
  of `JSTraceOp`.
- Update signatures of other class operations where needed, to use the
  Handle API.

### E4X ###

E4X support (inline XML in JS) is being phased out at this point.
If your code uses E4X, you'll need to pass `JSOPTION_ALLOW_XML` and 
possibly `JSOPTION_MOAR_XML` to `JS_SetOptions()` in order for it to
keep working.

However, E4X is a dead end in terms of standardization, and is removed
altogether in the very next version of SpiderMonkey, so the
recommendation is to migrate your code not to use it.
There isn't a one-size-fits-all replacement; some options are
transpilation, parsing the XML from a string, and writing new APIs that
take plain objects.

### GC callbacks ###

The old GC callback API has been split up.
GC callbacks are now called only when GC begins and ends, and is passed
a `JSGCStatus` value of `JSGC_BEGIN` or `JSGC_END`.
For the finalize phase, there is a separate callback API that is set
with `JS_SetFinalizeCallback()`, and is passed a `JSFinalizeStatus`
value of `JSFINALIZE_START` or `JSFINALIZE_END`.
This replaces the old `JSGCStatus` values of `JSGC_MARK_END` and
`JSGC_FINALIZE_END`.

`JS_SetGCCallback()` and `JS_SetGCCallbackRT()` have been combined into
a single `JS_SetGCCallback()` API that takes `JSRuntime*` instead of
`JSContext*`.

**Recommendation:**
If your GC callback performed any actions on `JSGC_MARK_END` or
`JSGC_FINALIZE_END`, you'll need to split that code out into a separate
finalize callback.
You may need to fix up some API calls that take `JSRuntime*` instead of
`JSContext*` (see also "Various API changes" below).
Use `JS_GetRuntime()` or `JS_GetObjectRuntime()` if you don't have a
pointer to the runtime already.

### Various API changes ###

This is a non-exhaustive list of minor API changes and renames.

- `JS_BufferIsCompilableUnit()` gets a boolean "bytes are UTF-8"
  argument in position 2.
  Previously, the bytes were not treated as UTF-8.
- `JS_CompileFile()` → `JS_CompileUTF8File()`
- `JS_CompileFileHandle()` → `JS_CompileUTF8FileHandle()`
- `JS_CompileFileHandleForPrinicpals()` →
  `JS_CompileUTF8FileHandleForPrincipals()`
- `JS_CompileFileHandleForPrinicpalsVersion()` →
  `JS_CompileUTF8FileHandleForPrincipalsVersion()`
- Several APIs now take a `JSRuntime*` instead of a `JSContext*`:
  - `JS_CompartmentGC()`
  - `JS_DumpHeap()`
  - `JS_GC()`
  - `JS_IsInRequest()`
  - `JS_SetNativeStackQuota()`
  - `JS_TracerInit()`
- Several APIs no longer take a `JSContext*` as the first argument:
  - `JS_GetClass()`
  - `JS_GetCompartmentPrivate()`
  - `JS_GetParent()`
  - `JS_GetPrivate()`
  - `JS_GetPrototype()`
  - `JS_GetReservedSlot()`
  - `JS_IsAboutToBeFinalized()`
  - `JS_SetCompartmentPrivate()`
  - `JS_SetPrivate()`
  - `JS_SetReservedSlot()`
- `JSIdArray` is now opaque.
  Instead of accessing the `.length` and `.vector` members, use
  `JS_IdArrayLength()` and `JS_IdArrayGet()`.
- `JS_NewCompartmentAndGlobalObject()` → `JS_NewGlobalObject()`
- `JS_NewObjectForConstructor()` now takes an additional `JSClass*`
  argument.
- `JS_Remove___Root()` functions no longer have a return value.
- `JSTraceCallback` is passed a `void**` pointer to the traced pointer,
  instead of the `void*` pointer itself.

The following APIs have been removed.

- `JSCLASS_CONCURRENT_FINALIZER`: objects with finalize hooks are never
  finalized on a background thread anymore.
- `JSCLASS_CONSTRUCT_PROTOTYPE`: this flag caused a class's constructor
  to be called once on its prototype object.
  If you need to do any initialization on the prototype object, do it
  after initializing the class instead. 
- `JS_ConstructObject()` and `JS_ConstructObjectWithArguments()`: use
  `JS_New()` instead, and pass a pointer to the constructor object.
  The old APIs would malfunction if the constructor was no longer
  reachable from the global object.
- `JS_DestroyContextMaybeGC()`: use either `JS_DestroyContext()` or
  `JS_DestroyContextNoGC()`.
- `JSFinalizeStub`: use null instead.
  (Despite the comment in `jsapi.h` that says `JSClass.finalize` must
  not be null!)
- `JS_FlushCaches()`: no replacement.
- `JS_GET_CLASS` macro: use `JS_GetClass()` instead.
- `JS_IsConstructing_PossiblyWithGivenThisObject()`: once no longer
  using `JS_ConstructObject()` and `JSCLASS_CONSTRUCT_PROTOTYPE`, this
  can be replaced with `JS_IsConstructing()`.
- `JS_IsScriptFrame()`: can be replaced by `JS_GetFrameScript()` which
  returns null if the frame is not a script frame.
- `JS::MarkRuntime()`: this was an internal function and was unlikely to
  be used by embeddings.
- `JS_NewNumberValue()`: use `JS_NumberValue()` instead, but note that
  the semantics are slightly different: it canonicalizes NaN, and it
  cannot fail.
- `JSOPTION_JIT`, `JSOPTION_PROFILING`: no replacement.
- `JSOPTION_SOFTEN`: Use `JS_SetJitHardening()` instead.
- The `JSPD_ARGUMENT` flag for property descriptors: in ESR 17 you can
  assume that it's always set.
- `JS_SetThreadStackLimit()`: use `JS_SetNativeStackQuota()` instead.
- `JSVAL_IS_OBJECT()`: a drop-in replacement is
  `JSVAL_IS_NULL(v) || !JSVAL_IS_PRIMITIVE(v)`, but the long-term
  recommendation is to migrate to the C++ `JS::Value` API (see above.)
  Note that `JSVAL_IS_OBJECT()` corresponds to
  `JS::Value::isObjectOrNull()`, not `JS::Value::isObject()`.
- `JS::Value::setObjectOrUndefined()`: open-code this if you need it.
