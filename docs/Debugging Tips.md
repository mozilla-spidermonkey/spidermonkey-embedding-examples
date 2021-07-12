# Debugging tips #

This page lists a few tips to help you investigate issues related to use
of the SpiderMonkey API.

Many tips listed here are dealing with the `js91` JavaScript shell built
during the SpiderMonkey build process.
If you want to implement these functions in your own code, you can look
at how they are implemented in the source code of the JS shell.

Many of these tips only apply to debug builds; they will not function in
a release build.

## Getting help from JS shell ##

Use the `help()` function to get the list of all primitive functions of
the shell with their description.
Note that some functions have been moved under an `os` object, and
`help(os)` will give brief help on just the members of that "namespace".
A regular expression can also be passed to `help()`, for example
`help(/gc/i)`.

## Getting the bytecode of a function ##

The shell has a small function named `dis()` to dump the bytecode of a
function with its source notes.
Without arguments, it will dump the bytecode of its caller.

```
js> function f () {
  return 1;
}
js> dis(f);
flags:
loc     op
-----   --
main:
00000:  one
00001:  return
00002:  stop

Source notes:
 ofs  line    pc  delta desc     args
---- ---- ----- ------ -------- ------
  0:    1     0 [   0] newline
  1:    2     0 [   0] colspan 2
  3:    2     2 [   2] colspan 9
```

In gdb, a function named `js::DisassembleAtPC()` can print the bytecode
of a script.
Some variants of this function such as `js::DumpScript()` are convenient
for debugging.

## Printing the JS stack from gdb ##

A function named `js::DumpBacktrace()` prints a backtrace à la gdb for
the JS stack.
The backtrace contains in the following order: the stack depth; the
interpreter frame pointer (see the `StackFrame` class) or `(nil)` if
compiled with IonMonkey; the file and line number of the call location;
and in parentheses, the `JSScript` pointer and the `jsbytecode` pointer
(PC) executed.

```
$ gdb --args js91
[…]
(gdb) b js::ReportOverRecursed
(gdb) r
js> function f(i) {
  if (i % 2) f(i + 1);
  else f(i + 3);
}
js> f(0)

Breakpoint 1, js::ReportOverRecursed (maybecx=0xfdca70) at /home/nicolas/mozilla/ionmonkey/js/src/jscntxt.cpp:495
495         if (maybecx)
(gdb) call js::DumpBacktrace(maybecx)
#0          (nil)   typein:2 (0x7fffef1231c0 @ 0)
#1          (nil)   typein:2 (0x7fffef1231c0 @ 24)
#2          (nil)   typein:3 (0x7fffef1231c0 @ 47)
#3          (nil)   typein:2 (0x7fffef1231c0 @ 24)
#4          (nil)   typein:3 (0x7fffef1231c0 @ 47)
[…]
#25157 0x7fffefbbc250   typein:2 (0x7fffef1231c0 @ 24)
#25158 0x7fffefbbc1c8   typein:3 (0x7fffef1231c0 @ 47)
#25159 0x7fffefbbc140   typein:2 (0x7fffef1231c0 @ 24)
#25160 0x7fffefbbc0b8   typein:3 (0x7fffef1231c0 @ 47)
#25161 0x7fffefbbc030   typein:5 (0x7fffef123280 @ 9)
```

Note, you can do the exact same exercise above using `lldb` (necessary
on macOS after Apple removed gdb) by running `lldb -f js91` then
following the remaining steps.

We also have a gdb unwinder.
This unwinder is able to read the frames created by the JIT, and to
display the frames which are after these JIT frames.

```
$ gdb --args out/dist/bin/js ./foo.js
[…]
SpiderMonkey unwinder is disabled by default, to enable it type:
        enable unwinder .* SpiderMonkey
(gdb) b js::math_cos
(gdb) run
[…]
#0  js::math_cos (cx=0x14f2640, argc=1, vp=0x7fffffff6a88) at js/src/jsmath.cpp:338
338         CallArgs args = CallArgsFromVp(argc, vp);
(gdb) enable unwinder .* SpiderMonkey
(gdb) backtrace 10
#0  0x0000000000f89979 in js::math_cos(JSContext*, unsigned int, JS::Value*) (cx=0x14f2640, argc=1, vp=0x7fffffff6a88) at js/src/jsmath.cpp:338
#1  0x0000000000ca9c6e in js::CallJSNative(JSContext*, bool (*)(JSContext*, unsigned int, JS::Value*), JS::CallArgs const&) (cx=0x14f2640, native=0xf89960 , args=...) at js/src/jscntxtinlines.h:235
#2  0x0000000000c87625 in js::Invoke(JSContext*, JS::CallArgs const&, js::MaybeConstruct) (cx=0x14f2640, args=..., construct=js::NO_CONSTRUCT) at js/src/vm/Interpreter.cpp:476
#3  0x000000000069bdcf in js::jit::DoCallFallback(JSContext*, js::jit::BaselineFrame*, js::jit::ICCall_Fallback*, uint32_t, JS::Value*, JS::MutableHandleValue) (cx=0x14f2640, frame=0x7fffffff6ad8, stub_=0x1798838, argc=1, vp=0x7fffffff6a88, res=JSVAL_VOID) at js/src/jit/BaselineIC.cpp:6113
#4  0x00007ffff7f41395 in <<JitFrame_Exit>> ()
#5  0x00007ffff7f42223 in <<JitFrame_BaselineStub>> ()
#6  0x00007ffff7f4423d in <<JitFrame_BaselineJS>> ()
#7  0x00007ffff7f4222e in <<JitFrame_BaselineStub>> ()
#8  0x00007ffff7f4326a in <<JitFrame_BaselineJS>> ()
#9  0x00007ffff7f38d5f in <<JitFrame_Entry>> ()
#10 0x00000000006a86de in EnterBaseline(JSContext*, js::jit::EnterJitData&) (cx=0x14f2640, data=...) at js/src/jit/BaselineJIT.cpp:150
```

Note, when you enable the unwinder, the current version of gdb (7.10.1)
does not flush the backtrace.
Therefore, the JIT frames do not appear until you settle on the next
breakpoint.
To work around this issue you can use the recording feature of gdb, to
step one instruction, and settle back to where you came from with the
following set of gdb commands:

```
(gdb) record full
(gdb) si
(gdb) record goto 0
(gdb) record stop
```

If you have a core file, you can use the gdb unwinder the same way, or
do everything from the command line as follows:

```
$ gdb -ex 'enable unwinder .* SpiderMonkey' -ex 'bt 0' -ex 'thread apply all backtrace' -ex 'quit' out/dist/bin/js corefile
```

The gdb unwinder is supposed to be loaded by `dist/bin/js-gdb.py` and
load python scripts which are located in `js/src/gdb/mozilla` under gdb.
If gdb does not load the unwinder by default, you can force it to, by
using the `source` command with the js-gdb.py file.

## Dumping the JavaScript heap ##

From the shell, you can call the `dumpHeap()` function to dump out all
GC things (reachable and unreachable) that are present in the heap.
By default the function writes to stdout, but a filename can be
specified as an argument.

Example output might look as follows:

```
0x1234abcd B global object
==========
# zone 0x56789123
# compartment http://gmail.com [in zone 0x56789123]
# compartment http://gmail.com/iframe [in zone 0x56789123]
# arena allockind=3 size=64
0x1234abcd B object
> 0x1234abcd B prop1
> 0xabcd1234 W prop2
0xabcd1234 W object
> 0xdeadbeef B prop3
# arena allockind=5 size=72
0xdeadbeef W object
> 0xabcd1234 W prop4
```

The output is textual.
The first section of the file contains a list of roots, one per line.
Each root has the form `0xabcd1234 <color> <description>`, where
`<color>` is the marking color of the given GC thing (`B` for black, `G`
for gray, `W` for white) and `<description>` is a string.
The list of roots ends with a line containing `==========`.

After the roots come a series of zones.
A zone starts with several "comment lines" that start with hashes.
The first comment declares the zone.
It is followed by lines listing each compartment within the zone.
After all the compartments come arenas, which is where the GC things are
actually stored.
Each arena is followed by all the GC things in the arena.
A GC thing starts with a line giving its address, its color, and the
thing kind (object, function, whatever).
After this come a list of addresses that the GC thing points to, each
one starting with `>`.

It's also possible to dump the JavaScript heap from C++ code (or from
gdb) using the `js::DumpHeap()` function.
It is part of `jsfriendapi.h` and it is available in release builds.

## Dumping garbage collection statistics ##

The environment variable `MOZ_GCTIMER` controls text dumping of GC
statistics.
`MOZ_GCTIMER` may be `none`, `stderr`, `stdout`, or a filename.
If a filename is given, data is appended to that file.

## Setting a breakpoint in the generated code from gdb ##

To set a breakpoint the generated code of a specific `JSScript` compiled
with IonMonkey, set a breakpoint on the instruction you are interested
in.
If you have no precise idea which function you are looking at, you can
set a breakpoint on the `js::ion::CodeGenerator::visitStart()` function.
Optionally, a condition on the `ins->id()` of the LIR instruction can be
added to select precisely the instruction you are looking for.
Once the breakpoint is on the `CodeGenerator` function of the LIR
instruction, add a command to generate a static breakpoint in the
generated code.

```
$ gdb --args js
[…]
(gdb) b js::ion::CodeGenerator::visitStart
(gdb) command
>call masm.breakpoint()
>continue
>end
(gdb) r
js> function f(a, b) { return a + b; }
js> for (var  i = 0; i < 100000; i++) f(i, i + 1);

Breakpoint 1, js::ion::CodeGenerator::visitStart (this=0x101ed20, lir=0x10234e0)
    at /home/nicolas/mozilla/ionmonkey/js/src/ion/CodeGenerator.cpp:609
609     }

Program received signal SIGTRAP, Trace/breakpoint trap.
0x00007ffff7fb165a in ?? ()
(gdb)
```

Once you hit the generated breakpoint, you can replace it by a gdb
breakpoint to make it conditional, the procedure is to first replace
the generated breakpoint by a nop instruction, and to set a breakpoint
at the address of the nop.

```
(gdb) x /5i $pc - 1
   0x7ffff7fb1659:      int3
=> 0x7ffff7fb165a:      mov    0x28(%rsp),%rax
   0x7ffff7fb165f:      mov    %eax,%ecx
   0x7ffff7fb1661:      mov    0x30(%rsp),%rdx
   0x7ffff7fb1666:      mov    %edx,%ebx

(gdb) # replace the int3 by a nop
(gdb) set *(unsigned char *) ($pc - 1) = 0x90
(gdb) x /1i $pc - 1
   0x7ffff7fb1659:      nop

(gdb) # set a breakpoint at the previous location
(gdb) b *0x7ffff7fb1659
Breakpoint 2 at 0x7ffff7fb1659
```

## Printing Ion generated assembly code from gdb ##

If you want to look at the assembly code generated by IonMonkey, you can follow this procedure:

- Place a breakpoint in `CodeGenerator.cpp` on the
  `CodeGenerator::link()` method.
- Step next a few times, so that the `code` variable gets generated.
- Print `code->code_`, which is the address of the code.
- Disassemble code read at this address (using `x/Ni address`, where _N_
  is the number of instructions you would like to see.)

Here is an example.
It might be simpler to use the `CodeGenerator::link()` line number
instead of the full qualified name to put the breakpoint.
Let's say that the line number of this function is 4780, for instance:

```
(gdb) b CodeGenerator.cpp:4780
Breakpoint 1 at 0x84cade0: file /home/code/mozilla-central/js/src/ion/CodeGenerator.cpp, line 4780.
(gdb) r
Starting program: /home/code/mozilla-central/js/src/32-release/js -f /home/code/jaeger.js
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib/x86_64-linux-gnu/libthread_db.so.1".
[New Thread 0xf7903b40 (LWP 12563)]
[New Thread 0xf6bdeb40 (LWP 12564)]
Run#0

Breakpoint 1, js::ion::CodeGenerator::link (this=0x86badf8)
    at /home/code/mozilla-central/js/src/ion/CodeGenerator.cpp:4780
4780    {
(gdb) n
4781        JSContext *cx = GetIonContext()->cx;
(gdb) n
4783        Linker linker(masm);
(gdb) n
4784        IonCode *code = linker.newCode(cx, JSC::ION_CODE);
(gdb) n
4785        if (!code)
(gdb) p code->code_
$1 = (uint8_t *) 0xf7fd25a8 "\201", <incomplete sequence \354\200>
(gdb) x/2i 0xf7fd25a8
   0xf7fd25a8:    sub    $0x80,%esp
   0xf7fd25ae:    mov    0x94(%esp),%ecx
```

On ARM, the compiled JS code will always be ARM machine code, whereas
SpiderMonkey itself is frequently Thumb2.
Since there isn't debug info for the JITted code, you will need to tell
gdb that you are looking at ARM code:

```
(gdb) set arm force-mode arm
```

Or you can wrap the `x` command in your own command:

```
def xi
    set arm force-mode arm
    eval "x/%di %d", $arg0, $arg1
    set arm force-mode auto
end
```

## Printing asm.js/wasm generated assembly code from gdb ##

- Set a breakpoint on `js::wasm::Instance::callExport()` (defined in
  `WasmInstance.cpp` as of November 18th 2016).
  This will trigger for _any_ asm.js/wasm call, so you should find a way
  to set this breakpoint for only the generated codes you want to look
  at.
- Run the program.
- Do `next` in gdb until you reach the definition of the `funcPtr`:
  ```
  // Call the per-exported-function trampoline created by GenerateEntry.
  auto funcPtr = JS_DATA_TO_FUNC_PTR(ExportFuncPtr, codeBase() + func.entryOffset());
  if (!CALL_GENERATED_2(funcPtr, exportArgs.begin(), &tlsData_))
      return false;
  ```
- After it's set, `x/64i funcPtr` will show you the trampoline code.
  There should be a call to an address at some point; that's what we're
  targeting.
  Copy that address.
  ```
  0x7ffff7ff6000:    push   %r15
  0x7ffff7ff6002:    push   %r14
  0x7ffff7ff6004:    push   %r13
  0x7ffff7ff6006:    push   %r12
  0x7ffff7ff6008:    push   %rbp
  0x7ffff7ff6009:    push   %rbx
  0x7ffff7ff600a:    movabs $0xea4f80,%r10
  0x7ffff7ff6014:    mov    0x178(%r10),%r10
  0x7ffff7ff601b:    mov    %rsp,0x40(%r10)
  0x7ffff7ff601f:    mov    (%rsi),%r15
  0x7ffff7ff6022:    mov    %rdi,%r10
  0x7ffff7ff6025:    push   %r10
  0x7ffff7ff6027:    test   $0xf,%spl
  0x7ffff7ff602b:    je     0x7ffff7ff6032
  0x7ffff7ff6031:    int3
  0x7ffff7ff6032:    callq  0x7ffff7ff5000     <------ right here
  ```
- `x/64i address` (in this case: `x/64i 0x7ffff7ff6032`).
- If you want to put a breakpoint at the function's entry, you can do:
  `b *address` (for instance here, `b* 0x7ffff7ff6032`).
  Then you can display the instructions around pc with `x/20i $pc`, and
  execute instruction by instruction with `stepi`.

