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

