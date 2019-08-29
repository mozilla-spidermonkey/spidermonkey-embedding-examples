# Building SpiderMonkey for Embedders #

Use these instructions to build your own copy of SpiderMonkey.

## Prerequisites ##

You will need a **C++ compiler** that can handle the C++14 standard,
**GNU Make**, **zlib**, and **libffi**.
These can usually be installed with a package manager.
You will also need **Autoconf 2.13** (not any later version) which is
also usually available through a package manager.
The package will usually be called something like `autoconf2.13` rather
than just `autoconf`.

> **NOTE** SpiderMonkey also requires ICU of at least version 59.1, but
> it will build a bundled copy by default.
> If you have a new enough copy installed on your system, you can add
> `--with-system-icu` in the build instructions below, for a shorter
> build time.

## Getting the source code ##

Currently, the most reliable way to get the SpiderMonkey source code is
to download the Firefox source.
At the time of writing, the latest source for Firefox ESR 68, which
contains the source for SpiderMonkey ESR 68, can be found here:
https://ftp.mozilla.org/pub/firefox/releases/68.2.0esr/source/

The ESR releases have a major release approximately once a year with
security patches released throughout the year.
It is recommended that embedders track ESR to have reasonable API
stability.
The master branch of SpiderMonkey experiences a fair amount of breaking
changes unfortunately, driven by the needs of the Firefox browser.

> **NOTE** Mozilla may be able to provide separate source packages for
> SpiderMonkey in the future, but this is difficult for a number of
> reasons.

## Building SpiderMonkey ##

First you should decide where you want to install SpiderMonkey.
By default, it will install into `/usr/local`.
You might want to pick some other location if `/usr/local` is not
writable to you without superuser permissions, for example.

```sh
cd js/src
mkdir _build
cd _build
../configure --disable-jemalloc  --enable-posix-nspr-emulation \
    --with-system-zlib --with-intl-api --enable-debug --enable-optimize
make
make install  # sudo if necessary
```

Add `--prefix=/my/prefix` (where `/my/prefix` is your chosen prefix) to
the `configure` line if you chose a different prefix.

If you are building a package for production, omit the `--enable-debug`.

If you picked a different prefix to install into, and that prefix is not
a standard place where libraries are loaded from, you may need to
execute the following when you want to use the SpiderMonkey libraries,
for example when building the examples from this repository.

```sh
export PKG_CONFIG_PATH=/my/prefix/lib/pkgconfig
export LD_LIBRARY_PATH=/my/prefix/lib
```

### Disabling jemalloc ###

One important configuration when getting started is the
`--disable-jemalloc` flag.
This will cause SpiderMonkey to use the system allocator functions
instead of a custom build of jemalloc.
The custom configuration, which is the default, is intended for a
browser environment and requires linking the final application with a
matching version of a library called mozglue.
If one accidentally builds SpiderMonkey for their embedding without
including the `--disable-jemalloc` flag, they usually quickly encounter
strange crashes as items allocated in jemalloc allocator are freed on
system allocator.
