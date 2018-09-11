# SpiderMonkey Embedding Examples #

## Prerequsisites ##

You need Meson 0.43.0 or later to build the examples.
Installation instructions for Meson are [here](https://mesonbuild.com/Getting-meson.html).

You will also need SpiderMonkey ESR 60 installed where Meson can find
it.
Generally this means that the `mozjs-60.pc` file needs to be installed
in a location known to pkg-config.

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
