# SpiderMonkey Embedding Examples #

## Prerequsisites ##

You need Meson 0.43.0 or later to build the examples.
Installation instructions for Meson are [here](https://mesonbuild.com/Getting-meson.html).

You will also need SpiderMonkey ESR 102 installed where Meson can find
it.
Generally this means that the `mozjs-102.pc` file needs to be installed
in a location known to pkg-config, and the `libmozjs-102.so` file needs
to be in the path for loading libraries.

Many Linux distributions have development packages for SpiderMonkey 102
and if you just want to try the examples, installing that is the easiest
way to get a build of SpiderMonkey.
If you are on macOS or Windows, or want to do any development, read the
[Building SpiderMonkey for Embedders](../docs/Building%20SpiderMonkey.md)
page.

For the REPL example, you will need readline installed where Meson can
find it, as well.

## To build ##

To compile these examples, build in the toplevel directory:
```sh
meson _build
ninja -C _build
```

## To contribute ##

Install the clang-format commit hook:

```sh
tools/git-pre-commit-format install
```

If adding a new example to the examples directory, make sure to build it
in the `meson.build` file, and add a description of it to this
`README.md` file.

## List of examples ##

- **hello.cpp** - Simple Hello World program, shows how to do the bare
  minimum to embed SpiderMonkey and execute a single line of JS code.
- **tracing.cpp** - Example of how to safely store pointers to
  garbage-collected things into your C++ data structures.
- **cookbook.cpp** - Based on an old wiki page called "JSAPI Cookbook",
  this program doesn't do anything in particular but contains a lot of
  examples showing how to do common operations with SpiderMonkey.
- **repl.cpp** - Best practices for creating a mini JavaScript
  interpreter, consisting of a read-eval-print loop.
- **resolve.cpp** - Best practices for creating a JS class that uses
  lazy property resolution.
  Use this in cases where defining properties and methods in your class
  upfront might be slow.
- **modules.cpp** - Example of how to load ES Module sources.
- **worker.cpp** - Example of how to use SpiderMonkey in multiple threads.
- **stencils.cpp** - Example of how to use Stencils to cache and reuse
  compiled scripts. The example demonstrates Stencils in multiple threads.

