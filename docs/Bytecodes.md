# Bytecode Listing #

This document is automatically generated from
[`Opcodes.h`](https://searchfox.org/mozilla-esr68/source/js/src/vm/Opcodes.h) by
[`make_opcode_doc.py`](../tools/make_opcode_doc.py).

## Background ##

SpiderMonkey bytecodes are the canonical form of code representation that is
used in the JavaScript engine.
The JavaScript frontend constructs an AST from the source text, then emits
stack-based bytecodes from that AST as a part of the `JSScript` data structure.
Bytecodes can reference atoms and objects (typically by array index) which are
also contained in the `JSScript` data structure.

Within the engine, all bytecode executed within a stack frame — even global
(top-level) and eval code — has a stack frame associated with it.
A frame on the stack has space for JavaScript Values (the tagged value format)
in a few different categories.
The space for a single JavaScript value is called a "slot", so the categories
are:

- Argument slots: holds the actual arguments passed to the current frame.
- Local slots: holds the local variables used by the current code.
- Expression slots: holds the temporary space that you need to calculate
  expressions on a stack. For example, in `(a + b) + c` you would push a, then
  push b, then add, then push c, then add, which requires a maximum depth of
  two expression slots.

There are also some slots reserved for dedicated functionality, holding values
like `this` and the callee / return value.

There is always a "Top of Stack" (TOS) that corresponds to the latest value
pushed onto the expression stack.
All bytecodes implicitly operate in terms of this location.

## Bytecode listing ##

All opcodes are annotated with a `[-popcount, +pushcount]` to represent the
overall stack-effects of their execution.


### Statements ###

#### Jumps ####



##### JSOP_AND [-1, +1] (JUMP, DETECTING, LEFTASSOC)

|                |     |
| -------------- | --- |
| **Value**      | `69 (0x45)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `cond` |
| **Stack Defs** | `cond` |

<p>Converts the top of stack value into a boolean, if the result is <code>false</code>,
jumps to a 32-bit offset from the current bytecode.
</p>

##### JSOP_BACKPATCH [-0, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `149 (0x95)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Placeholder opcode used during bytecode generation. This never
appears in a finished script. FIXME: bug 473671.
</p>

##### JSOP_GOTO [-0, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `6 (0x06)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Jumps to a 32-bit offset from the current bytecode.
</p>

##### JSOP_IFEQ [-1, +0] (JUMP, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `7 (0x07)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `cond` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value, converts it into a boolean, if the result is
<code>false</code>, jumps to a 32-bit offset from the current bytecode.
</p>
<p>The idea is that a sequence like
<code>JSOP_ZERO</code>; <code>JSOP_ZERO</code>; <code>JSOP_EQ</code>; <code>JSOP_IFEQ</code>; <code>JSOP_RETURN</code>;
reads like a nice linear sequence that will execute the return.
</p>

##### JSOP_IFNE [-1, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `8 (0x08)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `cond` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value, converts it into a boolean, if the result is
<code>true</code>, jumps to a 32-bit offset from the current bytecode.
</p>

##### JSOP_LABEL [-0, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `106 (0x6a)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>This opcode precedes every labeled statement. It's a no-op.
</p>
<p><code>offset</code> is the offset to the next instruction after this statement,
the one <code>break LABEL;</code> would jump to. IonMonkey uses this.
</p>

##### JSOP_LOOPENTRY [-0, +0] (UINT8)

|                |     |
| -------------- | --- |
| **Value**      | `227 (0xe3)` |
| **Operands**   | `uint8_t BITFIELD` |
| **Length**     | 2 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>This opcode is the target of the entry jump for some loop. The uint8
argument is a bitfield. The lower 7 bits of the argument indicate the
loop depth. This value starts at 1 and is just a hint: deeply nested
loops all have the same value.  The upper bit is set if Ion should be
able to OSR at this point, which is true unless there is non-loop state
on the stack.
</p>

##### JSOP_LOOPHEAD [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `109 (0x6d)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Another no-op.
</p>
<p>This opcode is the target of the backwards jump for some loop.
</p>

##### JSOP_OR [-1, +1] (JUMP, DETECTING, LEFTASSOC)

|                |     |
| -------------- | --- |
| **Value**      | `68 (0x44)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `cond` |
| **Stack Defs** | `cond` |

<p>Converts the top of stack value into a boolean, if the result is <code>true</code>,
jumps to a 32-bit offset from the current bytecode.
</p>

#### Switch Statement ####



##### JSOP_CASE [-2, +1] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `121 (0x79)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `lval(if lval !== rval)` |

<p>Pops the top two values on the stack as <code>rval</code> and <code>lval</code>, compare them
with <code>===</code>, if the result is <code>true</code>, jumps to a 32-bit offset from the
current bytecode, re-pushes <code>lval</code> onto the stack if <code>false</code>.
</p>

##### JSOP_CONDSWITCH [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `120 (0x78)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>This no-op appears after the bytecode for EXPR in <code>switch (EXPR) {...}</code>
if the switch cannot be optimized using <code>JSOP_TABLESWITCH</code>.
For a non-optimized switch statement like this:
</p>
<pre>   switch (EXPR) {
     case V0:
       C0;
     ...
     default:
       D;
   }
</pre>
<p>the bytecode looks like this:
</p>
<pre>   (EXPR)
   condswitch
   (V0)
   case -&gt;C0
   ...
   default -&gt;D
   (C0)
   ...
   (D)
</pre>
<p>Note that code for all case-labels is emitted first, then code for
the body of each case clause.
</p>

##### JSOP_DEFAULT [-1, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `122 (0x7a)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | `lval` |
| **Stack Defs** | ` ` |

<p>This appears after all cases in a <code>JSOP_CONDSWITCH</code>, whether there is a
<code>default:</code> label in the switch statement or not. Pop the switch operand
from the stack and jump to a 32-bit offset from the current bytecode.
offset from the current bytecode.
</p>

##### JSOP_TABLESWITCH [-1, +0] (TABLESWITCH, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `70 (0x46)` |
| **Operands**   | `int32_t len, int32_t low, int32_t high,int32_t offset[0], ..., int32_t offset[high-low]` |
| **Length**     | len |
| **Stack Uses** | `i` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value as <code>i</code>, if <code>low &lt;= i &lt;= high</code>,
jumps to a 32-bit offset: <code>offset[i - low]</code> from the current bytecode,
jumps to a 32-bit offset: <code>len</code> from the current bytecode if not.
</p>
<p>This opcode has variable length.
</p>

#### For-In Statement ####



##### JSOP_ENDITER [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `78 (0x4e)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `iter` |
| **Stack Defs** | ` ` |

<p>Exits a for-in loop by popping the iterator object from the stack and
closing it.
</p>

##### JSOP_ISNOITER [-1, +2]

|                |     |
| -------------- | --- |
| **Value**      | `77 (0x4d)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `val, res` |

<p>Pushes a boolean indicating whether the value on top of the stack is
MagicValue(<code>JS_NO_ITER_VALUE</code>).
</p>


##### JSOP_ITER [-1, +1] (UINT8)

|                |     |
| -------------- | --- |
| **Value**      | `75 (0x4b)` |
| **Operands**   | `uint8_t flags` |
| **Length**     | 2 |
| **Stack Uses** | `val` |
| **Stack Defs** | `iter` |

<p>Sets up a for-in or for-each-in loop using the <code>JSITER_*</code> flag bits in
this op's uint8_t immediate operand. It pops the top of stack value as
<code>val</code> and pushes <code>iter</code> which is an iterator for <code>val</code>.
</p>

##### JSOP_MOREITER [-1, +2]

|                |     |
| -------------- | --- |
| **Value**      | `76 (0x4c)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `iter` |
| **Stack Defs** | `iter, val` |

<p>Pushes the next iterated value onto the stack. If no value is available,
MagicValue(<code>JS_NO_ITER_VALUE</code>) is pushed.
</p>


#### With Statement ####



##### JSOP_ENTERWITH [-1, +0] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `3 (0x03)` |
| **Operands**   | `uint32_t staticWithIndex` |
| **Length**     | 5 |
| **Stack Uses** | `val` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value, converts it to an object, and adds a
<code>DynamicWithObject</code> wrapping that object to the scope chain.
</p>
<p>There is a matching <code>JSOP_LEAVEWITH</code> instruction later. All name
lookups between the two that may need to consult the With object
are deoptimized.
</p>

##### JSOP_LEAVEWITH [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `4 (0x04)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Pops the scope chain object pushed by <code>JSOP_ENTERWITH</code>.
</p>

#### Exception Handling ####



##### JSOP_EXCEPTION [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `118 (0x76)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `exception` |

<p>Pushes the current pending exception onto the stack and clears the
pending exception. This is only emitted at the beginning of code for a
catch-block, so it is known that an exception is pending. It is used to
implement catch-blocks and <code>yield*</code>.
</p>

##### JSOP_FINALLY [-0, +2]

|                |     |
| -------------- | --- |
| **Value**      | `135 (0x87)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `false, (next bytecode's PC)` |

<p>This opcode has a def count of 2, but these values are already on the
stack (they're pushed by <code>JSOP_GOSUB</code>).
</p>

##### JSOP_GOSUB [-0, +0] (JUMP)

|                |     |
| -------------- | --- |
| **Value**      | `116 (0x74)` |
| **Operands**   | `int32_t offset` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Pushes <code>false</code> and next bytecode's PC onto the stack, and jumps to
a 32-bit offset from the current bytecode.
</p>
<p>This opcode is used for entering <code>finally</code> block.
</p>

##### JSOP_RETSUB [-2, +0]

|                |     |
| -------------- | --- |
| **Value**      | `117 (0x75)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | ` ` |

<p>Pops the top two values on the stack as <code>rval</code> and <code>lval</code>, converts
<code>lval</code> into a boolean, raises error if the result is <code>true</code>,
jumps to a 32-bit absolute PC: <code>rval</code> if <code>false</code>.
</p>
<p>This opcode is used for returning from <code>finally</code> block.
</p>

##### JSOP_THROW [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `112 (0x70)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value as <code>v</code>, sets pending exception as <code>v</code>, then
raises error.
</p>

##### JSOP_THROWING [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `151 (0x97)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value as <code>v</code>, sets pending exception as <code>v</code>,
to trigger rethrow.
</p>
<p>This opcode is used in conditional catch clauses.
</p>

##### JSOP_TRY [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `134 (0x86)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>This no-op appears at the top of the bytecode for a <code>TryStatement</code>.
</p>
<p>Location information for catch/finally blocks is stored in a
side table, <code>script-&gt;trynotes()</code>.
</p>

#### Function ####



##### JSOP_CALL [-(argc+2), +1] (UINT16, INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `58 (0x3a)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>callee</code> with <code>this</code> and <code>args</code>, pushes return value onto the
stack.
</p>

##### JSOP_EVAL [-(argc+2), +1] (UINT16, INVOKE, TYPESET, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `123 (0x7b)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>eval</code> with <code>args</code> and pushes return value onto the stack.
</p>
<p>If <code>eval</code> in global scope is not original one, invokes the function
with <code>this</code> and <code>args</code>, and pushes return value onto the stack.
</p>

##### JSOP_FUNAPPLY [-(argc+2), +1] (UINT16, INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `79 (0x4f)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>callee</code> with <code>this</code> and <code>args</code>, pushes return value onto the
stack.
</p>
<p>This is for <code>f.apply</code>.
</p>

##### JSOP_FUNCALL [-(argc+2), +1] (UINT16, INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `108 (0x6c)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>callee</code> with <code>this</code> and <code>args</code>, pushes return value onto the
stack.
</p>
<p>If <code>callee</code> is determined to be the canonical <code>Function.prototype.call</code>
function, then this operation is optimized to directly call <code>callee</code>
with <code>args[0]</code> as <code>this</code>, and the remaining arguments as formal args
to <code>callee</code>.
</p>
<p>Like <code>JSOP_FUNAPPLY</code> but for <code>f.call</code> instead of <code>f.apply</code>.
</p>

##### JSOP_LAMBDA [-0, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `130 (0x82)` |
| **Operands**   | `uint32_t funcIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes a closure for a named or anonymous function expression onto the
stack.
</p>

##### JSOP_LAMBDA_ARROW [-1, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `131 (0x83)` |
| **Operands**   | `uint32_t funcIndex` |
| **Length**     | 5 |
| **Stack Uses** | `this` |
| **Stack Defs** | `obj` |

<p>Pops the top of stack value as <code>this</code>, pushes an arrow function with
<code>this</code> onto the stack.
</p>

##### JSOP_NEW [-(argc+2), +1] (UINT16, INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `82 (0x52)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>callee</code> as a constructor with <code>this</code> and <code>args</code>, pushes return
value onto the stack.
</p>

##### JSOP_RETRVAL [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `153 (0x99)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Stops interpretation and returns value set by <code>JSOP_SETRVAL</code>. When not set,
returns <code>undefined</code>.
</p>
<p>Also emitted at end of script so interpreter don't need to check if
opcode is still in script range.
</p>

##### JSOP_RETURN [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `5 (0x05)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `rval` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value as <code>rval</code>, stops interpretation of current
script and returns <code>rval</code>.
</p>

##### JSOP_RUNONCE [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `71 (0x47)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Prologue emitted in scripts expected to run once, which deoptimizes code
if it executes multiple times.
</p>

##### JSOP_SETCALL [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `74 (0x4a)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Sometimes web pages do <code>o.Item(i) = j</code>. This is not an early SyntaxError,
for web compatibility. Instead we emit <code>JSOP_SETCALL</code> after the function
call, an opcode that always throws.
</p>

##### JSOP_SETRVAL [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `152 (0x98)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `rval` |
| **Stack Defs** | ` ` |

<p>Pops the top of stack value as <code>rval</code>, sets the return value in stack
frame as <code>rval</code>.
</p>

##### JSOP_SPREADCALL [-3, +1] (INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `41 (0x29)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `callee, this, args` |
| **Stack Defs** | `rval` |

<p>spreadcall variant of <code>JSOP_CALL</code>.
</p>
<p>Invokes <code>callee</code> with <code>this</code> and <code>args</code>, pushes the return value onto
the stack.
</p>
<p><code>args</code> is an Array object which contains actual arguments.
</p>

##### JSOP_SPREADEVAL [-3, +1] (INVOKE, TYPESET, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `43 (0x2b)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `callee, this, args` |
| **Stack Defs** | `rval` |

<p>spreadcall variant of <code>JSOP_EVAL</code>
</p>
<p>Invokes <code>eval</code> with <code>args</code> and pushes the return value onto the stack.
</p>
<p>If <code>eval</code> in global scope is not original one, invokes the function
with <code>this</code> and <code>args</code>, and pushes return value onto the stack.
</p>

##### JSOP_SPREADNEW [-3, +1] (INVOKE, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `42 (0x2a)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `callee, this, args` |
| **Stack Defs** | `rval` |

<p>spreadcall variant of <code>JSOP_NEW</code>
</p>
<p>Invokes <code>callee</code> as a constructor with <code>this</code> and <code>args</code>, pushes the
return value onto the stack.
</p>

##### JSOP_STRICTEVAL [-(argc+2), +1] (UINT16, INVOKE, TYPESET, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `124 (0x7c)` |
| **Operands**   | `uint16_t argc` |
| **Length**     | 3 |
| **Stack Uses** | `callee, this, args[0], ..., args[argc-1]` |
| **Stack Defs** | `rval` |

<p>Invokes <code>eval</code> with <code>args</code> and pushes return value onto the stack.
</p>
<p>If <code>eval</code> in global scope is not original one, invokes the function
with <code>this</code> and <code>args</code>, and pushes return value onto the stack.
</p>

##### JSOP_STRICTSPREADEVAL [-3, +1] (INVOKE, TYPESET, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `50 (0x32)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `callee, this, args` |
| **Stack Defs** | `rval` |

<p>spreadcall variant of <code>JSOP_EVAL</code>
</p>
<p>Invokes <code>eval</code> with <code>args</code> and pushes the return value onto the stack.
</p>
<p>If <code>eval</code> in global scope is not original one, invokes the function
with <code>this</code> and <code>args</code>, and pushes return value onto the stack.
</p>

#### Generator ####



##### JSOP_FINALYIELDRVAL [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `204 (0xcc)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `gen` |
| **Stack Defs** | ` ` |

<p>Pops the generator and suspends and closes it. Yields the value in the
frame's return value slot.
</p>

##### JSOP_GENERATOR [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `201 (0xc9)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `generator` |

<p>Initializes generator frame, creates a generator and pushes it on the
stack.
</p>

##### JSOP_INITIALYIELD [-1, +1] (UINT24)

|                |     |
| -------------- | --- |
| **Value**      | `202 (0xca)` |
| **Operands**   | `uint24_t yieldIndex` |
| **Length**     | 4 |
| **Stack Uses** | `generator` |
| **Stack Defs** | `wat` |

<p>Pops the generator from the top of the stack, suspends it and stops
interpretation.
</p>

##### JSOP_RESUME [-2, +1] (UINT8, INVOKE)

|                |     |
| -------------- | --- |
| **Value**      | `205 (0xcd)` |
| **Operands**   | `resume kind (GeneratorObject::ResumeKind)` |
| **Length**     | 3 |
| **Stack Uses** | `gen, val` |
| **Stack Defs** | `rval` |

<p>Pops the generator and argument from the stack, pushes a new generator
frame and resumes execution of it. Pushes the return value after the
generator yields.
</p>

##### JSOP_YIELD [-2, +1] (UINT24)

|                |     |
| -------------- | --- |
| **Value**      | `203 (0xcb)` |
| **Operands**   | `uint24_t yieldIndex` |
| **Length**     | 4 |
| **Stack Uses** | `rval1, gen` |
| **Stack Defs** | `rval2` |

<p>Pops the generator and the return value <code>rval1</code>, stops interpretation and
returns <code>rval1</code>. Pushes sent value from <code>send()</code> onto the stack.
</p>

#### Debugger ####



##### JSOP_DEBUGGER [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `115 (0x73)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Invokes debugger.
</p>

##### JSOP_DEBUGLEAVEBLOCK [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `200 (0xc8)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>The opcode to assist the debugger.
</p>

### Variables and Scopes ###

#### Variables ####



##### JSOP_BINDNAME [-0, +1] (ATOM, NAME, SET)

|                |     |
| -------------- | --- |
| **Value**      | `110 (0x6e)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `scope` |

<p>Looks up name on the scope chain and pushes the scope which contains
the name onto the stack. If not found, pushes global scope onto the
stack.
</p>

##### JSOP_DEFCONST [-0, +0] (ATOM)

|                |     |
| -------------- | --- |
| **Value**      | `128 (0x80)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Defines the new binding on the frame's current variables-object (the
scope object on the scope chain designated to receive new variables) with
<code>READONLY</code> attribute. The binding is *not* <code>JSPROP_PERMANENT</code>. See bug
1019181 for the reason.
</p>
<p>This is used for global scripts and also in some cases for function
scripts where use of dynamic scoping inhibits optimization.
</p>

##### JSOP_DEFFUN [-0, +0] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `127 (0x7f)` |
| **Operands**   | `uint32_t funcIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Defines the given function on the current scope.
</p>
<p>This is used for global scripts and also in some cases for function
scripts where use of dynamic scoping inhibits optimization.
</p>

##### JSOP_DEFVAR [-0, +0] (ATOM)

|                |     |
| -------------- | --- |
| **Value**      | `129 (0x81)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Defines the new binding on the frame's current variables-object (the
scope object on the scope chain designated to receive new variables).
</p>
<p>This is used for global scripts and also in some cases for function
scripts where use of dynamic scoping inhibits optimization.
</p>

##### JSOP_DELNAME [-0, +1] (ATOM, NAME, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `36 (0x24)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `succeeded` |

<p>Looks up name on the scope chain and deletes it, pushes <code>true</code> onto the
stack if succeeded (if the property was present and deleted or if the
property wasn't present in the first place), <code>false</code> if not.
</p>
<p>Strict mode code should never contain this opcode.
</p>

##### JSOP_GETNAME [-0, +1] (ATOM, NAME, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `59 (0x3b)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Looks up name on the scope chain and pushes its value onto the stack.
</p>

##### JSOP_SETCONST [-1, +1] (ATOM, NAME, SET)

|                |     |
| -------------- | --- |
| **Value**      | `14 (0x0e)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `val` |
| **Stack Defs** | `val` |

<p>Defines a readonly property on the frame's current variables-object (the
scope object on the scope chain designated to receive new variables).
</p>

##### JSOP_SETNAME [-2, +1] (ATOM, NAME, SET, DETECTING, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `111 (0x6f)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `scope, val` |
| **Stack Defs** | `val` |

<p>Pops a scope and value from the stack, assigns value to the given name,
and pushes the value back on the stack
</p>

##### JSOP_STRICTSETNAME [-2, +1] (ATOM, NAME, SET, DETECTING, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `49 (0x31)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `scope, val` |
| **Stack Defs** | `val` |

<p>Pops a scope and value from the stack, assigns value to the given name,
and pushes the value back on the stack. If the set failed, then throw
a TypeError, per usual strict mode semantics.
</p>

#### Free Variables ####



##### JSOP_BINDGNAME [-0, +1] (ATOM, NAME, SET, GNAME)

|                |     |
| -------------- | --- |
| **Value**      | `214 (0xd6)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `global` |

<p>Pushes the global scope onto the stack.
</p>
<p><code>nameIndex</code> is not used.
</p>

##### JSOP_GETGNAME [-0, +1] (ATOM, NAME, TYPESET, GNAME)

|                |     |
| -------------- | --- |
| **Value**      | `154 (0x9a)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Looks up name on global scope and pushes its value onto the stack.
</p>
<p>Free variable references that must either be found on the global or a
ReferenceError.
</p>

##### JSOP_SETGNAME [-2, +1] (ATOM, NAME, SET, DETECTING, GNAME, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `155 (0x9b)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `scope, val` |
| **Stack Defs** | `val` |

<p>Pops the top two values on the stack as <code>val</code> and <code>scope</code>, sets property
of <code>scope</code> as <code>val</code> and pushes <code>val</code> back on the stack.
</p>
<p><code>scope</code> should be the global scope.
</p>

##### JSOP_STRICTSETGNAME [-2, +1] (ATOM, NAME, SET, DETECTING, GNAME, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `156 (0x9c)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `scope, val` |
| **Stack Defs** | `val` |

<p>Pops the top two values on the stack as <code>val</code> and <code>scope</code>, sets property
of <code>scope</code> as <code>val</code> and pushes <code>val</code> back on the stack. Throws a
TypeError if the set fails, per strict mode semantics.
</p>
<p><code>scope</code> should be the global scope.
</p>

#### Local Variables ####



##### JSOP_CHECKLEXICAL [-0, +0] (LOCAL, NAME)

|                |     |
| -------------- | --- |
| **Value**      | `138 (0x8a)` |
| **Operands**   | `uint32_t localno` |
| **Length**     | 4 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Checks if the value of the local variable is the
<code>JS_UNINITIALIZED_LEXICAL</code> magic, throwing an error if so.
</p>

##### JSOP_GETLOCAL [-0, +1] (LOCAL, NAME)

|                |     |
| -------------- | --- |
| **Value**      | `86 (0x56)` |
| **Operands**   | `uint32_t localno` |
| **Length**     | 4 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes the value of local variable onto the stack.
</p>

##### JSOP_INITLEXICAL [-1, +1] (LOCAL, NAME, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `139 (0x8b)` |
| **Operands**   | `uint32_t localno` |
| **Length**     | 4 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v` |

<p>Initializes an uninitialized local lexical binding with the top of stack
value.
</p>

##### JSOP_SETLOCAL [-1, +1] (LOCAL, NAME, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `87 (0x57)` |
| **Operands**   | `uint32_t localno` |
| **Length**     | 4 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v` |

<p>Stores the top stack value to the given local.
</p>

#### Aliased Variables ####



##### JSOP_CHECKALIASEDLEXICAL [-0, +0] (SCOPECOORD, NAME)

|                |     |
| -------------- | --- |
| **Value**      | `140 (0x8c)` |
| **Operands**   | `uint8_t hops, uint24_t slot` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Checks if the value of the aliased variable is the
<code>JS_UNINITIALIZED_LEXICAL</code> magic, throwing an error if so.
</p>

##### JSOP_GETALIASEDVAR [-0, +1] (SCOPECOORD, NAME, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `136 (0x88)` |
| **Operands**   | `uint8_t hops, uint24_t slot` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `aliasedVar` |

<p>Pushes aliased variable onto the stack.
</p>
<p>An "aliased variable" is a var, let, or formal arg that is aliased.
Sources of aliasing include: nested functions accessing the vars of an
enclosing function, function statements that are conditionally executed,
<code>eval</code>, <code>with</code>, and <code>arguments</code>. All of these cases require creating a
CallObject to own the aliased variable.
</p>
<p>An ALIASEDVAR opcode contains the following immediates:
</p>
<pre>uint8 hops:  the number of scope objects to skip to find the ScopeObject
             containing the variable being accessed
uint24 slot: the slot containing the variable in the ScopeObject (this
             'slot' does not include RESERVED_SLOTS).
</pre>

##### JSOP_INITALIASEDLEXICAL [-1, +1] (SCOPECOORD, NAME, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `141 (0x8d)` |
| **Operands**   | `uint8_t hops, uint24_t slot` |
| **Length**     | 5 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v` |

<p>Initializes an uninitialized aliased lexical binding with the top of
stack value.
</p>

##### JSOP_SETALIASEDVAR [-1, +1] (SCOPECOORD, NAME, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `137 (0x89)` |
| **Operands**   | `uint8_t hops, uint24_t slot` |
| **Length**     | 5 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v` |

<p>Sets aliased variable as the top of stack value.
</p>

#### Intrinsics ####



##### JSOP_BINDINTRINSIC [-0, +1] (ATOM, NAME, SET)

|                |     |
| -------------- | --- |
| **Value**      | `145 (0x91)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `intrinsicHolder` |

<p>Pushes <code>intrinsicHolder</code> onto the stack.
</p>

##### JSOP_GETINTRINSIC [-0, +1] (ATOM, NAME, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `143 (0x8f)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `intrinsic[name]` |

<pre>Pushes the value of the intrinsic onto the stack.
</pre>
<p>Intrinsic names are emitted instead of <code>JSOP_*NAME</code> ops when the
<code>CompileOptions</code> flag <code>selfHostingMode</code> is set.
</p>
<p>They are used in self-hosted code to access other self-hosted values and
intrinsic functions the runtime doesn't give client JS code access to.
</p>

##### JSOP_SETINTRINSIC [-2, +1] (ATOM, NAME, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `144 (0x90)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `scope, val` |
| **Stack Defs** | `val` |

<p>Pops the top two values on the stack as <code>val</code> and <code>scope</code>, sets intrinsic
as <code>val</code>, and pushes <code>val</code> onto the stack.
</p>
<p><code>scope</code> is not used.
</p>

#### Block-local Scope ####



##### JSOP_POPBLOCKSCOPE [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `199 (0xc7)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Pops block from the scope chain.
</p>

##### JSOP_PUSHBLOCKSCOPE [-0, +0] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `198 (0xc6)` |
| **Operands**   | `uint32_t staticBlockObjectIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Pushes block onto the scope chain.
</p>

#### This ####



##### JSOP_IMPLICITTHIS [-0, +1] (ATOM)

|                |     |
| -------------- | --- |
| **Value**      | `226 (0xe2)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `this` |

<p>Pushes the implicit <code>this</code> value for calls to the associated name onto
the stack.
</p>

##### JSOP_THIS [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `65 (0x41)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `this` |

<p>Pushes <code>this</code> value for current stack frame onto the stack.
</p>

#### Arguments ####



##### JSOP_ARGUMENTS [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `9 (0x09)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `arguments` |

<p>Pushes the <code>arguments</code> object for the current function activation.
</p>
<p>If <code><code>JSS</code>cript</code> is not marked <code>needsArgsObj</code>, then a
<code>JS_OPTIMIZED_ARGUMENTS</code> magic value is pushed. Otherwise, a proper
arguments object is constructed and pushed.
</p>
<p>This opcode requires that the function does not have rest parameter.
</p>

##### JSOP_CALLEE [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `132 (0x84)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `callee` |

<p>Pushes current callee onto the stack.
</p>
<p>Used for named function expression self-naming, if lightweight.
</p>

##### JSOP_GETARG [-0, +1] (QARG , NAME)

|                |     |
| -------------- | --- |
| **Value**      | `84 (0x54)` |
| **Operands**   | `uint16_t argno` |
| **Length**     | 3 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `arguments[argno]` |

<p>Fast get op for function arguments and local variables.
</p>
<p>Pushes <code>arguments[argno]</code> onto the stack.
</p>

##### JSOP_REST [-0, +1] (TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `224 (0xe0)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `rest` |

<p>Creates rest parameter array for current function call, and pushes it
onto the stack.
</p>

##### JSOP_SETARG [-1, +1] (QARG , NAME, SET)

|                |     |
| -------------- | --- |
| **Value**      | `85 (0x55)` |
| **Operands**   | `uint16_t argno` |
| **Length**     | 3 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v` |

<p>Fast set op for function arguments and local variables.
</p>
<p>Sets <code>arguments[argno]</code> as the top of stack value.
</p>

### Operators ###

#### Comparison Operators ####



##### JSOP_EQ [-2, +1] (LEFTASSOC, ARITH, DETECTING)
##### JSOP_GE [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_GT [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_LE [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_LT [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_NE [-2, +1] (LEFTASSOC, ARITH, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_EQ: 18 (0x12)`<br>`JSOP_GE: 23 (0x17)`<br>`JSOP_GT: 22 (0x16)`<br>`JSOP_LE: 21 (0x15)`<br>`JSOP_LT: 20 (0x14)`<br>`JSOP_NE: 19 (0x13)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval OP rval)` |

<p>Pops the top two values from the stack and pushes the result of
comparing them.
</p>

##### JSOP_STRICTEQ [-2, +1] (DETECTING, LEFTASSOC, ARITH)
##### JSOP_STRICTNE [-2, +1] (DETECTING, LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_STRICTEQ: 72 (0x48)`<br>`JSOP_STRICTNE: 73 (0x49)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval OP rval)` |

<p>Pops the top two values from the stack, then pushes the result of
applying the operator to the two values.
</p>

#### Arithmetic Operators ####



##### JSOP_ADD [-2, +1] (LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `27 (0x1b)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval + rval)` |

<p>Pops the top two values <code>lval</code> and <code>rval</code> from the stack, then pushes
the result of <code>lval + rval</code>.
</p>

##### JSOP_DIV [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_MOD [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_MUL [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_SUB [-2, +1] (LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_DIV: 30 (0x1e)`<br>`JSOP_MOD: 31 (0x1f)`<br>`JSOP_MUL: 29 (0x1d)`<br>`JSOP_SUB: 28 (0x1c)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval OP rval)` |

<p>Pops the top two values <code>lval</code> and <code>rval</code> from the stack, then pushes
the result of applying the arithmetic operation to them.
</p>

##### JSOP_NEG [-1, +1] (ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `34 (0x22)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(-val)` |

<p>Pops the value <code>val</code> from the stack, then pushes <code>-val</code>.
</p>

##### JSOP_POS [-1, +1] (ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `35 (0x23)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(+val)` |

<p>Pops the value <code>val</code> from the stack, then pushes <code>+val</code>.
(<code>+val</code> is the value converted to a number.)
</p>

#### Bitwise Logical Operators ####



##### JSOP_BITAND [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_BITOR [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_BITXOR [-2, +1] (LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_BITAND: 17 (0x11)`<br>`JSOP_BITOR: 15 (0x0f)`<br>`JSOP_BITXOR: 16 (0x10)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval OP rval)` |

<p>Pops the top two values <code>lval</code> and <code>rval</code> from the stack, then pushes
the result of the operation applied to the two operands, converting
both to 32-bit signed integers if necessary.
</p>

##### JSOP_BITNOT [-1, +1] (ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `33 (0x21)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(~val)` |

<p>Pops the value <code>val</code> from the stack, then pushes <code>~val</code>.
</p>

#### Bitwise Shift Operators ####



##### JSOP_LSH [-2, +1] (LEFTASSOC, ARITH)
##### JSOP_RSH [-2, +1] (LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_LSH: 24 (0x18)`<br>`JSOP_RSH: 25 (0x19)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval OP rval)` |

<p>Pops the top two values <code>lval</code> and <code>rval</code> from the stack, then pushes
the result of the operation applied to the operands.
</p>

##### JSOP_URSH [-2, +1] (LEFTASSOC, ARITH)

|                |     |
| -------------- | --- |
| **Value**      | `26 (0x1a)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `lval, rval` |
| **Stack Defs** | `(lval &gt;&gt;&gt; rval)` |

<p>Pops the top two values <code>lval</code> and <code>rval</code> from the stack, then pushes
<code>lval &gt;&gt;&gt; rval</code>.
</p>

#### Logical Operators ####



##### JSOP_NOT [-1, +1] (ARITH, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `32 (0x20)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(!val)` |

<p>Pops the value <code>val</code> from the stack, then pushes <code>!val</code>.
</p>

#### Special Operators ####



##### JSOP_DELELEM [-2, +1] (BYTE , ELEM, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `38 (0x26)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval` |
| **Stack Defs** | `succeeded` |

<p>Pops the top two values on the stack as <code>propval</code> and <code>obj</code>,
deletes <code>propval</code> property from <code>obj</code>, pushes <code>true</code>  onto the stack if
succeeded, <code>false</code> if not.
</p>

##### JSOP_DELPROP [-1, +1] (ATOM, PROP, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `37 (0x25)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `succeeded` |

<p>Pops the top of stack value, deletes property from it, pushes <code>true</code> onto
the stack if succeeded, <code>false</code> if not.
</p>

##### JSOP_IN [-2, +1] (LEFTASSOC)

|                |     |
| -------------- | --- |
| **Value**      | `113 (0x71)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `id, obj` |
| **Stack Defs** | `(id in obj)` |

<p>Pops the top two values <code>id</code> and <code>obj</code> from the stack, then pushes
<code>id in obj</code>.  This will throw a <code>TypeError</code> if <code>obj</code> is not an object.
</p>
<p>Note that <code>obj</code> is the top value.
</p>

##### JSOP_INSTANCEOF [-2, +1] (LEFTASSOC, TMPSLOT)

|                |     |
| -------------- | --- |
| **Value**      | `114 (0x72)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, ctor` |
| **Stack Defs** | `(obj instanceof ctor)` |

<p>Pops the top two values <code>obj</code> and <code>ctor</code> from the stack, then pushes
<code>obj instanceof ctor</code>.  This will throw a <code>TypeError</code> if <code>obj</code> is not an
object.
</p>

##### JSOP_STRICTDELPROP [-1, +1] (ATOM, PROP, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `46 (0x2e)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `succeeded` |

<p>Pops the top of stack value and attempts to delete the given property
from it. Pushes <code>true</code> onto success, else throws a TypeError per strict
mode property-deletion requirements.
</p>

##### JSOP_TYPEOF [-1, +1] (DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `39 (0x27)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(typeof val)` |

<p>Pops the value <code>val</code> from the stack, then pushes <code>typeof val</code>.
</p>

##### JSOP_TYPEOFEXPR [-1, +1] (DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `197 (0xc5)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `(typeof val)` |

<p>Pops the top stack value as <code>val</code> and pushes <code>typeof val</code>.  Note that
this opcode isn't used when, in the original source code, <code>val</code> is a
name -- see <code><code>JSOP_TYPEOF</code></code> for that.
(This is because <code>typeof undefinedName === "undefined"</code>.)
</p>

##### JSOP_VOID [-1, +1]

|                |     |
| -------------- | --- |
| **Value**      | `40 (0x28)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `undefined` |

<p>Pops the top value on the stack and pushes <code>undefined</code>.
</p>

#### Stack Operations ####



##### JSOP_DUP [-1, +2]

|                |     |
| -------------- | --- |
| **Value**      | `12 (0x0c)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v` |
| **Stack Defs** | `v, v` |

<p>Pushes a copy of the top value on the stack.
</p>

##### JSOP_DUP2 [-2, +4]

|                |     |
| -------------- | --- |
| **Value**      | `13 (0x0d)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v1, v2` |
| **Stack Defs** | `v1, v2, v1, v2` |

<p>Duplicates the top two values on the stack.
</p>

##### JSOP_DUPAT [-0, +1] (UINT24)

|                |     |
| -------------- | --- |
| **Value**      | `44 (0x2c)` |
| **Operands**   | `uint24_t n` |
| **Length**     | 4 |
| **Stack Uses** | `v[n], v[n-1], ..., v[1], v[0]` |
| **Stack Defs** | `v[n], v[n-1], ..., v[1], v[0], v[n]` |

<p>Duplicates the Nth value from the top onto the stack.
</p>

##### JSOP_PICK [-0, +0] (UINT8, TMPSLOT2)

|                |     |
| -------------- | --- |
| **Value**      | `133 (0x85)` |
| **Operands**   | `uint8_t n` |
| **Length**     | 2 |
| **Stack Uses** | `v[n], v[n-1], ..., v[1], v[0]` |
| **Stack Defs** | `v[n-1], ..., v[1], v[0], v[n]` |

<p>Picks the nth element from the stack and moves it to the top of the
stack.
</p>

##### JSOP_POP [-1, +0]

|                |     |
| -------------- | --- |
| **Value**      | `81 (0x51)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v` |
| **Stack Defs** | ` ` |

<p>Pops the top value off the stack.
</p>

##### JSOP_POPN [-n, +0] (UINT16)

|                |     |
| -------------- | --- |
| **Value**      | `11 (0x0b)` |
| **Operands**   | `uint16_t n` |
| **Length**     | 3 |
| **Stack Uses** | `v[n-1], ..., v[1], v[0]` |
| **Stack Defs** | ` ` |

<p>Pops the top <code>n</code> values from the stack.
</p>

##### JSOP_SWAP [-2, +2]

|                |     |
| -------------- | --- |
| **Value**      | `10 (0x0a)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v1, v2` |
| **Stack Defs** | `v2, v1` |

<p>Swaps the top two values on the stack. This is useful for things like
post-increment/decrement.
</p>

#### Debugger ####



##### JSOP_DEBUGAFTERYIELD [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `208 (0xd0)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Bytecode emitted after <code>yield</code> expressions to help the Debugger
fix up the frame in the JITs. No-op in the interpreter.
</p>


### Literals ###

#### Constants ####



##### JSOP_DOUBLE [-0, +1] (DOUBLE)

|                |     |
| -------------- | --- |
| **Value**      | `60 (0x3c)` |
| **Operands**   | `uint32_t constIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes numeric constant onto the stack.
</p>

##### JSOP_FALSE [-0, +1]
##### JSOP_TRUE [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `JSOP_FALSE: 66 (0x42)`<br>`JSOP_TRUE: 67 (0x43)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `true/false` |

<p>Pushes boolean value onto the stack.
</p>

##### JSOP_INT32 [-0, +1] (INT32)

|                |     |
| -------------- | --- |
| **Value**      | `216 (0xd8)` |
| **Operands**   | `int32_t val` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes 32-bit int immediate integer operand onto the stack.
</p>

##### JSOP_INT8 [-0, +1] (INT8)

|                |     |
| -------------- | --- |
| **Value**      | `215 (0xd7)` |
| **Operands**   | `int8_t val` |
| **Length**     | 2 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes 8-bit int immediate integer operand onto the stack.
</p>

##### JSOP_NULL [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `64 (0x40)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `null` |

<p>Pushes <code>null</code> onto the stack.
</p>

##### JSOP_ONE [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `63 (0x3f)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `1` |

<p>Pushes <code>1</code> onto the stack.
</p>

##### JSOP_STRING [-0, +1] (ATOM)

|                |     |
| -------------- | --- |
| **Value**      | `61 (0x3d)` |
| **Operands**   | `uint32_t atomIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `string` |

<p>Pushes string constant onto the stack.
</p>

##### JSOP_SYMBOL [-0, +1] (UINT8)

|                |     |
| -------------- | --- |
| **Value**      | `45 (0x2d)` |
| **Operands**   | `uint8_t n, the JS::SymbolCode of the symbol to use` |
| **Length**     | 2 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `symbol` |

<p>Push a well-known symbol onto the operand stack.
</p>

##### JSOP_UINT16 [-0, +1] (UINT16)

|                |     |
| -------------- | --- |
| **Value**      | `88 (0x58)` |
| **Operands**   | `uint16_t val` |
| **Length**     | 3 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes unsigned 16-bit int immediate integer operand onto the stack.
</p>

##### JSOP_UINT24 [-0, +1] (UINT24)

|                |     |
| -------------- | --- |
| **Value**      | `188 (0xbc)` |
| **Operands**   | `uint24_t val` |
| **Length**     | 4 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `val` |

<p>Pushes unsigned 24-bit int immediate integer operand onto the stack.
</p>

##### JSOP_UNDEFINED [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `1 (0x01)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `undefined` |

<p>Pushes <code>undefined</code> onto the stack.
</p>

##### JSOP_UNINITIALIZED [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `142 (0x8e)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `uninitialized` |

<p>Pushes a <code>JS_UNINITIALIZED_LEXICAL</code> value onto the stack, representing an
uninitialized lexical binding.
</p>
<p>This opcode is used with the <code>JSOP_INITLET</code> opcode.
</p>

##### JSOP_ZERO [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `62 (0x3e)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `0` |

<p>Pushes <code>0</code> onto the stack.
</p>

#### Object ####



##### JSOP_CALLELEM [-2, +1] (BYTE , ELEM, TYPESET, LEFTASSOC)

|                |     |
| -------------- | --- |
| **Value**      | `193 (0xc1)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval` |
| **Stack Defs** | `obj[propval]` |

<p>Pops the top two values on the stack as <code>propval</code> and <code>obj</code>, pushes
<code>propval</code> property of <code>obj</code> onto the stack.
</p>
<p>Like <code>JSOP_GETELEM</code> but for call context.
</p>

##### JSOP_CALLPROP [-1, +1] (ATOM, PROP, TYPESET, TMPSLOT3)

|                |     |
| -------------- | --- |
| **Value**      | `184 (0xb8)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `obj[name]` |

<p>Pops the top of stack value, pushes property of it onto the stack.
</p>
<p>Like <code>JSOP_GETPROP</code> but for call context.
</p>

##### JSOP_CALLSITEOBJ [-0, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `101 (0x65)` |
| **Operands**   | `uint32_t objectIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes the call site object specified by objectIndex onto the stack. Defines the raw
property specified by objectIndex + 1 on the call site object and freezes both the call site
object as well as its raw property.
</p>

##### JSOP_GETELEM [-2, +1] (BYTE , ELEM, TYPESET, LEFTASSOC)

|                |     |
| -------------- | --- |
| **Value**      | `55 (0x37)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval` |
| **Stack Defs** | `obj[propval]` |

<p>Pops the top two values on the stack as <code>propval</code> and <code>obj</code>, pushes
<code>propval</code> property of <code>obj</code> onto the stack.
</p>

##### JSOP_GETPROP [-1, +1] (ATOM, PROP, TYPESET, TMPSLOT3)

|                |     |
| -------------- | --- |
| **Value**      | `53 (0x35)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `obj[name]` |

<p>Pops the top of stack value, pushes property of it onto the stack.
</p>

##### JSOP_GETXPROP [-1, +1] (ATOM, PROP, TYPESET)

|                |     |
| -------------- | --- |
| **Value**      | `195 (0xc3)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `obj[name]` |

<p>Pops the top of stack value, gets an extant property value of it,
throwing ReferenceError if the identified property does not exist.
</p>

##### JSOP_INITELEM [-3, +1] (ELEM, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `94 (0x5e)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, id, val` |
| **Stack Defs** | `obj` |

<p>Initialize a numeric property in an object literal, like <code>{1: x}</code>.
</p>
<p>Pops the top three values on the stack as <code>val</code>, <code>id</code> and <code>obj</code>, defines
<code>id</code> property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITELEM_GETTER [-3, +1] (ELEM, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `99 (0x63)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, id, val` |
| **Stack Defs** | `obj` |

<p>Initialize a numeric getter in an object literal like
<code>{get 2() {}}</code>.
</p>
<p>Pops the top three values on the stack as <code>val</code>, <code>id</code> and <code>obj</code>, defines
<code>id</code> getter of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITELEM_SETTER [-3, +1] (ELEM, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `100 (0x64)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, id, val` |
| **Stack Defs** | `obj` |

<p>Initialize a numeric setter in an object literal like
<code>{set 2(v) {}}</code>.
</p>
<p>Pops the top three values on the stack as <code>val</code>, <code>id</code> and <code>obj</code>, defines
<code>id</code> setter of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITPROP [-2, +1] (ATOM, PROP, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `93 (0x5d)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `obj` |

<p>Initialize a named property in an object literal, like <code>{a: x}</code>.
</p>
<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code>, defines
<code>nameIndex</code> property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITPROP_GETTER [-2, +1] (ATOM, PROP, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `97 (0x61)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `obj` |

<p>Initialize a getter in an object literal.
</p>
<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code>, defines getter
of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITPROP_SETTER [-2, +1] (ATOM, PROP, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `98 (0x62)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `obj` |

<p>Initialize a setter in an object literal.
</p>
<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code>, defines setter
of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_MUTATEPROTO [-2, +1]

|                |     |
| -------------- | --- |
| **Value**      | `194 (0xc2)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, newProto` |
| **Stack Defs** | `succeeded` |

<p><code>__proto__: v</code> inside an object initializer.
</p>
<p>Pops the top two values on the stack as <code>newProto</code> and <code>obj</code>, sets
prototype of <code>obj</code> as <code>newProto</code>, pushes <code>true</code> onto the stack if
succeeded, <code>false</code> if not.
</p>

##### JSOP_NEWINIT [-0, +1] (UINT8)

|                |     |
| -------------- | --- |
| **Value**      | `89 (0x59)` |
| **Operands**   | `uint8_t kind (, uint24_t extra)` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes newly created object onto the stack.
</p>
<p>This opcode takes the kind of initializer (<code>JSP</code>roto_Array or
<code>JSP</code>roto_Object).
</p>
<p>This opcode has an extra byte so it can be exchanged with <code>JSOP_NEWOBJECT</code>
during emit.
</p>

##### JSOP_NEWOBJECT [-0, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `91 (0x5b)` |
| **Operands**   | `uint32_t baseobjIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes newly created object onto the stack.
</p>
<p>This opcode takes an object with the final shape, which can be set at
the start and slots then filled in directly.
</p>

##### JSOP_OBJECT [-0, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `80 (0x50)` |
| **Operands**   | `uint32_t objectIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes deep-cloned object literal or singleton onto the stack.
</p>

##### JSOP_SETELEM [-3, +1] (BYTE , ELEM, SET, DETECTING, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `56 (0x38)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval, val` |
| **Stack Defs** | `val` |

<p>Pops the top three values on the stack as <code>val</code>, <code>propval</code> and <code>obj</code>,
sets <code>propval</code> property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the
stack.
</p>

##### JSOP_SETPROP [-2, +1] (ATOM, PROP, SET, DETECTING, CHECKSLOPPY)

|                |     |
| -------------- | --- |
| **Value**      | `54 (0x36)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `val` |

<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code> and performs
<code>obj.prop = val</code>, pushing <code>val</code> back onto the stack.
</p>

##### JSOP_STRICTDELELEM [-2, +1] (ELEM, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `47 (0x2f)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval` |
| **Stack Defs** | `succeeded` |

<p>Pops the top two values on the stack as <code>propval</code> and <code>obj</code>,
and attempts to delete <code>propval</code> property from <code>obj</code>. Pushes <code>true</code> onto
the stack on success, else throws a TypeError per strict mode property
deletion requirements.
</p>

##### JSOP_STRICTSETELEM [-3, +1] (BYTE , ELEM, SET, DETECTING, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `57 (0x39)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, propval, val` |
| **Stack Defs** | `val` |

<p>Pops the top three values on the stack as <code>val</code>, <code>propval</code> and <code>obj</code>,
sets <code>propval</code> property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the
stack. Throws a TypeError if the set fails, per strict mode
semantics.
</p>

##### JSOP_STRICTSETPROP [-2, +1] (ATOM, PROP, SET, DETECTING, CHECKSTRICT)

|                |     |
| -------------- | --- |
| **Value**      | `48 (0x30)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `val` |

<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code>, and performs
<code>obj.prop = val</code>, pushing <code>val</code> back onto the stack. Throws a TypeError
if the set-operation failed (per strict mode semantics).
</p>

##### JSOP_TOID [-1, +1]

|                |     |
| -------------- | --- |
| **Value**      | `225 (0xe1)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `id` |
| **Stack Defs** | `(jsid of id)` |

<p>Pops the top of stack value, converts it into a jsid (int or string), and
pushes it onto the stack.
</p>

#### Array ####



##### JSOP_ARRAYPUSH [-2, +0]

|                |     |
| -------------- | --- |
| **Value**      | `206 (0xce)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `v, obj` |
| **Stack Defs** | ` ` |

<p>Pops the top two values on the stack as <code>obj</code> and <code>v</code>, pushes <code>v</code> to
<code>obj</code>.
</p>
<p>This opcode is used for Array Comprehension.
</p>

##### JSOP_HOLE [-0, +1]

|                |     |
| -------------- | --- |
| **Value**      | `218 (0xda)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `hole` |

<p>Pushes a <code>JS_ELEMENTS_HOLE</code> value onto the stack, representing an omitted
property in an array literal (e.g. property 0 in the array <code>[, 1]</code>).
</p>
<p>This opcode is used with the <code>JSOP_NEWARRAY</code> opcode.
</p>

##### JSOP_INITELEM_ARRAY [-2, +1] (UINT24, ELEM, SET, DETECTING)

|                |     |
| -------------- | --- |
| **Value**      | `96 (0x60)` |
| **Operands**   | `uint24_t index` |
| **Length**     | 4 |
| **Stack Uses** | `obj, val` |
| **Stack Defs** | `obj` |

<p>Initialize an array element.
</p>
<p>Pops the top two values on the stack as <code>val</code> and <code>obj</code>, sets <code>index</code>
property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> onto the stack.
</p>

##### JSOP_INITELEM_INC [-3, +2] (ELEM, SET)

|                |     |
| -------------- | --- |
| **Value**      | `95 (0x5f)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `obj, index, val` |
| **Stack Defs** | `obj, (index + 1)` |

<p>Pops the top three values on the stack as <code>val</code>, <code>index</code> and <code>obj</code>, sets
<code>index</code> property of <code>obj</code> as <code>val</code>, pushes <code>obj</code> and <code>index + 1</code> onto
the stack.
</p>
<p>This opcode is used in Array literals with spread and spreadcall
arguments.
</p>

##### JSOP_LENGTH [-1, +1] (ATOM, PROP, TYPESET, TMPSLOT3)

|                |     |
| -------------- | --- |
| **Value**      | `217 (0xd9)` |
| **Operands**   | `uint32_t nameIndex` |
| **Length**     | 5 |
| **Stack Uses** | `obj` |
| **Stack Defs** | `obj['length']` |

<p>Pops the top of stack value, pushes the <code>length</code> property of it onto the
stack.
</p>

##### JSOP_NEWARRAY [-0, +1] (UINT24)

|                |     |
| -------------- | --- |
| **Value**      | `90 (0x5a)` |
| **Operands**   | `uint24_t length` |
| **Length**     | 4 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes newly created array onto the stack.
</p>
<p>This opcode takes the final length, which is preallocated.
</p>

##### JSOP_NEWARRAY_COPYONWRITE [-0, +1] (OBJECT)

|                |     |
| -------------- | --- |
| **Value**      | `102 (0x66)` |
| **Operands**   | `uint32_t objectIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `obj` |

<p>Pushes a newly created array onto the stack, whose elements are the same
as that of a template object's copy on write elements.
</p>


#### RegExp ####



##### JSOP_REGEXP [-0, +1] (REGEXP)

|                |     |
| -------------- | --- |
| **Value**      | `160 (0xa0)` |
| **Operands**   | `uint32_t regexpIndex` |
| **Length**     | 5 |
| **Stack Uses** | ` ` |
| **Stack Defs** | `regexp` |

<p>Pushes a regular expression literal onto the stack.
It requires special "clone on exec" handling.
</p>

### Other ###



##### JSOP_FORCEINTERPRETER [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `207 (0xcf)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>No-op bytecode only emitted in some self-hosted functions. Not handled by
the JITs so the script always runs in the interpreter.
</p>


##### JSOP_LINENO [-0, +0] (UINT16)

|                |     |
| -------------- | --- |
| **Value**      | `119 (0x77)` |
| **Operands**   | `uint32_t lineno` |
| **Length**     | 3 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>Embedded lineno to speedup <code>pc-&gt;line</code> mapping.
</p>

##### JSOP_NOP [-0, +0]

|                |     |
| -------------- | --- |
| **Value**      | `0 (0x00)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | ` ` |
| **Stack Defs** | ` ` |

<p>No operation is performed.
</p>

##### JSOP_TOSTRING [-1, +1]

|                |     |
| -------------- | --- |
| **Value**      | `228 (0xe4)` |
| **Operands**   | ` ` |
| **Length**     | 1 |
| **Stack Uses** | `val` |
| **Stack Defs** | `ToString(val)` |

<p>Converts the value on the top of the stack to a String
</p>

