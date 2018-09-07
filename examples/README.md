# SpiderMonkey Embedding Examples #

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
