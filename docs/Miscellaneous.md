# Unicode #

To pass Unicode data between JavaScript and native code, represent the
data in UTF-16 in memory.
JavaScript strings, property names, and programs are all made up of
`char16_t`s.

Many JSAPI functions operate on null-terminated, 8-bit `char` strings.
These functions convert their `char*` arguments to 16-bit strings by
zero-extending each 8-bit `char` to 16 bits.

The JSAPI provides `char16_t`-based versions of many API functions that
operate on strings, object properties, and JavaScript code.

`char16_t`-based functions work exactly like their `char`-based
namesakes, except that where traditional functions take a `char*`
argument, the Unicode versions take a `char16_t*` argument, usually with
a separate argument specifying the length of the string in `char16_t`s.

# Compiled scripts #

The easiest way to run a script is to use `JS::Evaluate`, which
compiles and executes the script in one go.

But sometimes an application needs to run a script many times. In this
case, it may be faster to compile the script once and execute it
multiple times.

The JSAPI provides a type, `JSScript`, that represents a compiled
script.
The life cycle of a `JSScript` looks like this:

- The application compiles some JavaScript code using `JS::Compile`.
  This function returns a pointer to a new `JSScript`.
- The application calls `JS_ExecuteScript`
  any number of times. It is safe to use a `JSScript` in multiple
  different contexts, but only within the `JSContext` and global in
  which it was created.

Here is some example code using a compiled script:

```c++
/*
 * Compile a script and execute it repeatedly until an
 * error occurs.  (If this ever returns, it returns false.
 * If there's no error it just keeps going.)
 */
bool
compileAndRepeat(JSContext* cx, const char* filename)
{
    JS::RootedScript script(cx);

    JS::CompileOptions options;
    options.setUTF8(true);
    if (!JS::Compile(cx, options, filename, &script))
        return false;   /* compilation error */

    for (;;) {
        JS::RootedValue result(cx);

        if (!JS_ExecuteScript(cx, script, &result))
            break;
        JS_MaybeGC(cx);
    }

    return false;
}
```

# Security #

Many applications use SpiderMonkey to run untrusted code.
In designing this kind of application, it's important to think through
the security concerns ahead of time.
Your application will need to do several things.

- **Deploy security updates** - Firefox automatically installs updates,
  so security fixes are deployed as soon as they are available.
  Unless you also regularly deploy SpiderMonkey security updates, a
  determined hacker could use publicly known bugs in the engine to
  attack your application.
  Note that the kind of attack we're talking about here is where a
  hacker uses JavaScript to attack the C++ code of the engine itself (or
  your embedding).
  The rest of the items in this list talk about security issues that
  arise within JavaScript itself, even if the engine is working properly.
- **Block simple denial-of-service attacks** - A program like
  `while(true){}` should not hang your application.
  To stop execution of scripts that run too long, use
  `JS_AddInterruptCallback`.
  Likewise, a function like `function f(){f();}` should not crash your
  application with a stack overflow.
  To block that, use `JS_SetNativeStackQuota`.
- **Control access to sensitive data** - Your application might expose
  data to some scripts that other scripts should not be able to see.
  For example, you might let your customers write scripts that operate
  on their own data, but not other customers' data.
  These access rules must be enforced somehow.
- **Control access to dangerous functionality** - Suppose your
  application has a method `deleteUserAccount()` which is meant to be
  used by administrators only.
  Obviously if untrusted code can use that method, you have a security
  problem.
