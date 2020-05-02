In order to run any JavaScript code in SpiderMonkey, an application must
have two key elements: a `JSContext`, and a global object.
This section describes what these things are.
The next section explains how to set them up, using JSAPI functions.

**Contexts.** A `JSContext`, or _context_, is like a little machine that
can do many things involving JavaScript code and objects.
It can compile and execute scripts, get and set object properties, call
JavaScript functions, convert JavaScript data from one type to another,
create objects, and so on.
Almost all JSAPI functions require a `JSContext*` as the first argument.
Use `JS_NewContext` and `JS_DestroyContext` to create and destroy a
context.

Use `JS_SetContextPrivate` and `JS_GetContextPrivate` to associate
application-specific data with a context.

When your application is done with SpiderMonkey altogether, use
`JS_ShutDown` to free any remaining cached resources.
(This is a mere nicety if the process is about to exit anyway. But as
that is not always the case, calling `JS_Shutdown` is a good habit to
get into.)

**Global objects.** Then, the _global object_ contains all the
classes, functions, and variables that are available for JavaScript code
to use.
Whenever JavaScript code does something like
`window.open("http://www.mozilla.org/")`, it is accessing a global
property, in this case `window`.
JSAPI applications have full control over what global properties scripts
can see.
The application starts out by creating an object and populating it with
the standard JavaScript classes, like `Array` and `Object`.
Then it adds whatever custom classes, functions, and variables (like
`window`) the application wants to provide; see [Custom
objects](#custom-objects) below.
Each time the application runs a JS script (using, for example,
`JS::Evaluate`), it provides the global object for that script to
use.
As the script runs, it can create global functions and variables of its
own.
All of these functions, classes, and variables are stored as properties
of the global object.
To create this object, you first need a `JSClass` with the
`JSCLASS_GLOBAL_FLAGS` option.
The example `hello.cpp` defines a very basic `JSClass` (named
`globalClass`) with no methods or properties of its own.
Use `JS_NewGlobalObject` to create a global object.
Use `JS_InitStandardClasses` to populate it with the standard JavaScript
globals.

# Executing C++ code in JavaScript #

C++ functions that can be called by JavaScript code have the type
`JSNative`, which is a typedef for
`bool (*)(JSContext* cx, unsigned argc, JS::Value* vp`.
Each `JSNative` has the same signature, regardless of what arguments it
expects to receive from JavaScript.

The JavaScript arguments to the function are given in `argc` and `vp`.
This signature is obsolescent and in the future will be replaced with
a `JS::CallArgs& args` parameter.
For now, the first line of your `JSNative` function should always be

```c++
JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
```

`args` functions as an array of those arguments.
The arguments do not have native C++ types like `int` and `float`;
rather, they are `JS::Value`s, JavaScript values.
The native function uses conversion functions such as `JS::ToNumber()`
to convert the arguments
to C++ types and store them in local variables.
The native function must store its JavaScript return value in
`args.rval()`.

On success, a `JSNative` must call `args.rval()` and return `true`.
The value stored in `args.rval()` is returned to the JavaScript caller.

On failure, a `JSNative` calls an error-reporting function, e.g.
`JS_ReportErrorUTF8`, and returns `false`.
This causes a JavaScript exception to be thrown.
The caller can catch the exception using a JavaScript `try/catch`
statement.

To make native functions callable from JavaScript, declare a table of
`JSFunctionSpec`s describing the functions.
Then call `JS_DefineFunctions`.

```c++
static JSFunctionSpec myjs_global_functions[] = {
    JS_FN("rand",   myjs_rand,   0, 0),
    JS_FN("srand",  myjs_srand,  0, 0),
    JS_FN("system", myjs_system, 1, 0),
    JS_FS_END
};

    ...
    if (!JS_DefineFunctions(cx, global, myjs_global_functions))
        return false;
    ...
```

Once the functions are defined in `global`, any script that uses
`global` as the global object can call them, just as any web page can
call `alert`.
