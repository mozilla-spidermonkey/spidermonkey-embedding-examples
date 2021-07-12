#!/usr/bin/env -S python3 -B
# coding: utf-8
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

""" Usage: make_opcode_doc.py PATH_TO_SPIDERMONKEY_SOURCE

    This script generates SpiderMonkey bytecode documentation
    from js/src/vm/Opcodes.h.

    Output is written to docs/Bytecode.md.
"""

import sys
import os
from xml.sax.saxutils import escape

SOURCE_BASE = 'https://searchfox.org/mozilla-esr91/source'

FORMAT_TO_IGNORE = {
    "JOF_BYTE",
    "JOF_UINT8",
    "JOF_UINT16",
    "JOF_UINT24",
    "JOF_UINT32",
    "JOF_INT8",
    "JOF_INT32",
    "JOF_TABLESWITCH",
    "JOF_REGEXP",
    "JOF_DOUBLE",
    "JOF_LOOPHEAD",
    "JOF_BIGINT",
}


def format_format(format):
    format = [flag for flag in format if flag not in FORMAT_TO_IGNORE]
    if len(format) == 0:
        return ''
    return '**Format:** {format}\n'.format(format=', '.join(format))


def maybe_escape(value, format_str, fallback=""):
    if value:
        return format_str.format(escape(value))
    return fallback


OPCODE_FORMAT = """\
##### {names}

{operands}
{stack}
{desc}
{format}\
"""


def print_opcode(opcode, out):
    opcodes = [opcode] + opcode.group
    names = ', '.join(maybe_escape(code.op, "`{}`") for code in opcodes)
    operands = maybe_escape(opcode.operands, "**Operands:** `({})`\n")
    stack_uses = maybe_escape(opcode.stack_uses, "`{}` ")
    stack_defs = maybe_escape(opcode.stack_defs, " `{}`")
    if stack_uses or stack_defs:
        stack = "**Stack:** {}&rArr;{}\n".format(stack_uses, stack_defs)
    else:
        stack = ""

    print(OPCODE_FORMAT.format(
        id=opcodes[0].op,
        names=names,
        operands=operands,
        stack=stack,
        desc=opcode.desc,
        format=format_format(opcode.format_),
    ), file=out)


id_cache = dict()
id_count = dict()


def print_doc(index, out):
    print("""# Bytecode Listing #

This document is automatically generated from
[`Opcodes.h`]({source_base}/js/src/vm/Opcodes.h) by
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


""".format(source_base=SOURCE_BASE), file=out)

    for (category_name, types) in index:
        print('### {name} ###\n'.format(name=category_name), file=out)
        for (type_name, opcodes) in types:
            if type_name:
                print('#### {name} ####\n'.format(name=type_name), file=out)
            print('\n', file=out)
            for opcode in opcodes:
                print_opcode(opcode, out)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: make_opcode_doc.py PATH_TO_SPIDERMONKEY_SOURCE",
              file=sys.stderr)
        sys.exit(1)
    dir = sys.argv[1]

    thisdir = os.path.dirname(os.path.realpath(__file__))
    sys.path.insert(0, thisdir)
    import jsopcode

    try:
        index, _ = jsopcode.get_opcodes(dir)
    except Exception as e:
        print("Error: {}".format(' '.join(map(str, e.args))), file=sys.stderr)
        sys.exit(1)

    with open(os.path.join(thisdir, '..', 'docs', 'Bytecodes.md'), 'w') as out:
        print_doc(index, out)
