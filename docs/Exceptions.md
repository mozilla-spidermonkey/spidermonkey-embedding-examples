# Errors and exceptions #

Almost every JSAPI function that takes a `JSContext*` argument can fail.
The system might run out of memory.
There might be a syntax error in a script.
Or a script might explicitly `throw` an exception.

The JavaScript language has exceptions, and C++ has exceptions, but they
are not the same thing.
SpiderMonkey does not use C++ exceptions for anything.
JSAPI functions never throw C++ exceptions, and when SpiderMonkey calls
an application callback, the callback must not throw a C++ exception.

See the `ReportError`, `ThrowValue`, and `ThrowError` examples in
the [JSAPI Cookbook](../examples/cookbook.cpp) that show how to work
with JavaScript exceptions in C++.

## Uncatchable errors ##

Another way for a `JSNative` callback to report an error is like this:

```c++
if (!p) {
    JS_ReportOutOfMemory(cx);
    return false;
}
```

This does something subtly different from what `JS_ReportErrorUTF8` and
similar functions do.

Most errors, including those raised by `JS_ReportErrorUTF8`, are
represented
as JavaScript exceptions and thus interact with the JavaScript
exception-handling language features, `try`, `catch`, and `finally`.
However, in some cases we do not want scripts to be able to `catch` an
error; we want script execution to terminate right away.
If the system runs out of memory in the middle of a script, we do not
want `finally` blocks to execute, because almost anything a script does
requires at least a little memory, and we have none.
If a script has been running too long and we want to kill it, it's no
good to throw an exception—the script could just `catch` it and keep
going.

Therefore `JS_ReportOutOfMemory(cx)` does _not_ set the pending
exception.
It is an uncatchable error.

If SpiderMonkey runs out of memory, or a JSAPI callback returns `false`
without an exception pending, this is treated as an uncatchable error.
The JavaScript stack is unwound in the normal way except that `catch`
and `finally` blocks are ignored.
The most recent JSAPI call returns `false` or `nullptr` to the application.

An uncatchable error leaves the `JSContext` in a good state.
It can be used again right away. The application does not have to do
anything to “recover” from the error, as far as the JSAPI is concerned.
(Of course, if the error is that the system is out of memory, that
problem remains to be dealt with.)

Here is some example code that throws an uncatchable error.

```c++
Log("The server room is on fire!");

/* Make sure the error is uncatchable. */
JS_ClearPendingException(cx);
return false;
```
