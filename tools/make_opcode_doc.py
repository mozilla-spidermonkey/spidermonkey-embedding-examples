#!/usr/bin/env python -B
# coding: utf-8

""" Usage: make_opcode_doc.py PATH_TO_SPIDERMONKEY_SOURCE

    This script generates SpiderMonkey bytecode documentation
    from js/src/vm/Opcodes.h.

    Output is written to docs/Bytecode.md.
"""

import sys
import os
from xml.sax.saxutils import escape

SOURCE_BASE = 'https://searchfox.org/mozilla-esr68/source'


def override(value, override_value):
    if override_value != '':
        return override_value

    return value


def format_flags(flags):
    flags = list(filter(lambda x: x != 'JOF_BYTE', flags))
    if len(flags) == 0:
        return ''

    flags = map(lambda x: x.replace('JOF_', ''), flags)
    return ' ({flags})'.format(flags=', '.join(flags))


def print_opcode(opcode, out):
    names_template = '{name} [-{nuses}, +{ndefs}]{flags}'
    opcodes = sorted([opcode] + opcode.group,
                     key=lambda opcode: opcode.name)
    names = map(lambda code: names_template.format(name=escape(code.name),
                                                   nuses=override(code.nuses,
                                                                  opcode.nuses_override),
                                                   ndefs=override(code.ndefs,
                                                                  opcode.ndefs_override),
                                                   flags=format_flags(code.flags)),
                opcodes)
    if len(opcodes) == 1:
        values = ['{value} (0x{value:02x})'.format(value=opcode.value)]
    else:
        values_template = '{name}: {value} (0x{value:02x})'
        values = map(lambda code: values_template.format(name=escape(code.name),
                                                         value=code.value),
                     opcodes)

    print("""##### {names}

|                |     |
| -------------- | --- |
| **Value**      | `{values}` |
| **Operands**   | `{operands}` |
| **Length**     | {length} |
| **Stack Uses** | `{stack_uses}` |
| **Stack Defs** | `{stack_defs}` |

{desc}
""".format(names='\n##### '.join(names),
           values='`<br>`'.join(values),
           operands=escape(opcode.operands) or " ",
           length=escape(override(opcode.length,
                                  opcode.length_override)),
           stack_uses=escape(opcode.stack_uses) or " ",
           stack_defs=escape(opcode.stack_defs) or " ",
           desc=opcode.desc),  # desc is already escaped
          file=out)


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

## Bytecode listing ##

All opcodes are annotated with a `[-popcount, +pushcount]` to represent the
overall stack-effects of their execution.

""".format(source_base=SOURCE_BASE), file=out)

    for (category_name, types) in index:
        print('### {name} ###\n'.format(name=category_name), file=out)
        for (type_name, opcodes) in types:
            if type_name:
                print('#### {name} ####\n'.format(name=type_name), file=out)
            print('\n', file=out)
            for opcode_ in sorted(opcodes,
                                  key=lambda opcode: opcode.sort_key):
                print_opcode(opcode_, out)


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
        print("Error: {}".format(e.args[0]), file=sys.stderr)
        sys.exit(1)

    with open(os.path.join(thisdir, '..', 'docs', 'Bytecodes.md'), 'w') as out:
        print_doc(index, out)
