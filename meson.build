project('spidermonkey-embedding-examples', 'cpp', version: 'esr90',
    meson_version: '>= 0.43.0',
    default_options: ['cpp_std=c++17', 'warning_level=3'])

cxx = meson.get_compiler('cpp')

args = ['-std=c++17', '-Wall']

zlib = dependency('zlib')  # (is already a SpiderMonkey dependency)
spidermonkey = dependency('mozjs')
readline = cxx.find_library('readline')

# Check if SpiderMonkey was compiled with --enable-debug. If this is the case,
# you must compile all your sources with -DDEBUG=1.
# See https://bugzilla.mozilla.org/show_bug.cgi?id=1261161
nondebug_spidermonkey = cxx.compiles('''
#include <js-config.h>
#ifdef JS_DEBUG
#error debug yes, if we did not already error out due to DEBUG not being defined
#endif
''',
    args: args, dependencies: spidermonkey,
    name: 'SpiderMonkey is a non-debug build')

if not nondebug_spidermonkey
    args += '-DDEBUG=1'
endif

# Check if a minimal SpiderMonkey program compiles, links, and runs. If not,
# it's most likely the case that SpiderMonkey was configured incorrectly, for
# example by building mozglue as a shared library.
minimal_program = cxx.run('''
#include <js/Initialization.h>
int main(void) {
    if (!JS_Init()) return 1;
    JS_ShutDown();
    return 0;
}
''',
    args: args, dependencies: spidermonkey,
    name: 'SpiderMonkey sanity check')

if not minimal_program.compiled() or minimal_program.returncode() != 0
    error('''
A minimal SpiderMonkey program could not be compiled, linked, or run. Most
likely you should build it with a different configuration. Check the recommended
configuration in this repository.''')
endif

add_project_arguments(args, language: 'cpp')

if cxx.get_id() == 'gcc' or cxx.get_id() == 'clang'
    test_warning_args = [
        # it's useful not to have to initialize all struct fields when defining a
        # JSClass, since some of them are padding.
        '-Wno-missing-field-initializers',

        # we use argument names in some cases for explanatory purposes
        '-Wno-unused-parameter',
    ]
else
    test_warning_args = []
endif

add_project_arguments(cxx.get_supported_arguments(test_warning_args),
    language: 'cpp')

executable('hello', 'examples/hello.cpp', 'examples/boilerplate.cpp', dependencies: spidermonkey)
executable('cookbook', 'examples/cookbook.cpp', 'examples/boilerplate.cpp', dependencies: spidermonkey)
executable('repl', 'examples/repl.cpp', 'examples/boilerplate.cpp', dependencies: [spidermonkey, readline])
executable('tracing', 'examples/tracing.cpp', 'examples/boilerplate.cpp', dependencies: spidermonkey)
executable('resolve', 'examples/resolve.cpp', 'examples/boilerplate.cpp', dependencies: [spidermonkey, zlib])
executable('modules', 'examples/modules.cpp', 'examples/boilerplate.cpp', dependencies: [spidermonkey])
