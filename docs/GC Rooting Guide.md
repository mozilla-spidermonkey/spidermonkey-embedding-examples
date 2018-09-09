## Introduction ##

This guide explains the basics of interacting with SpiderMonkey's
GC as a SpiderMonkey API user.
Since SpiderMonkey has a moving GC, it is very important that it knows
about each and every pointer to a GC thing in the system.
SpiderMonkey's rooting API tries to make this task as simple as
possible.

## What is a GC thing pointer? ##

"GC thing" is the term used to refer to memory allocated and managed by
the SpiderMonkey garbage collector.
The main types of GC thing pointer are:

- `JS::Value`
- `JSObject*`
- `JSString*`
- `JSScript*`
- `jsid`
- `JSFunction*`
- `JS::Symbol*`

Note that `JS::Value` and `jsid` can contain pointers internally even
though they are not a normal pointer type, hence their inclusion in this
list.

If you use these types directly, or create classes, structs or arrays
that contain them, you must follow the rules set out in this guide.
If you do not your program will not work correctly — if it works at all.

## GC things on the stack ##

### `JS::Rooted<T>` ###

All GC thing pointers stored **on the stack** (i.e., local variables and
parameters to functions) must use the `JS::Rooted<T>` class.
This is a template class where the template parameter is the type of the
GC thing it contains.
From the user perspective, a `JS::Rooted<T>` instance behaves exactly as
if it were the underlying pointer.

`JS::Rooted` must be constructed with a `JSContext*`, and optionally an initial value.

There are typedefs available for the main types.
Within SpiderMonkey, it is suggested that these are used in preference
to the template class (Gecko uses the template versions):

| Template class            | Typedef              |
| ------------------------- | -------------------- |
| `JS::Rooted<JS::Value>`   | `JS::RootedValue`    |
| `JS::Rooted<JSObject*>`   | `JS::RootedObject`   |
| `JS::Rooted<JSString*>`   | `JS::RootedString`   |
| `JS::Rooted<JSScript*>`   | `JS::RootedScript`   |
| `JS::Rooted<jsid>`        | `JS::RootedId`       |
| `JS::Rooted<JSFunction*>` | `JS::RootedFunction` |
| `JS::Rooted<JS::Symbol*>` | `JS::RootedSymbol`   |

For example, instead of this:

```c++
JSObject* localObj = JS_GetObjectOfSomeSort(cx);
```

You would write this:

```c++
JS::RootedObject localObj(cx, JS_GetObjectOfSomeSort(cx));
```

SpiderMonkey makes it easy to remember to use `JS::Rooted<T>` types
instead of a raw pointer because all of the API methods that may GC take
a `JS::Handle<T>`, as described below, and `JS::Rooted<T>` autoconverts
to `JS::Handle<T>` but a bare pointer does not.

### `JS::Handle<T>` ###

All GC thing pointers that are parameters to a function must be wrapped
in `JS::Handle<T>`. A `JS::Handle<T>` is a reference to a
`JS::Rooted<T>`, and is created implicitly by referencing a
`JS::Rooted<T>`: It is not valid to create a `JS::Handle<T>` manually
(the whole point of a Handle is that it only reference pointers that the
GC knows about so it can update them when they move).
Like `JS::Rooted<T>`, a `JS::Handle<T>` can be used as if it were the
underlying pointer.

Since only a `JS::Rooted<T>` will cast to a `JS::Handle<T>`, the
compiler will enforce correct rooting of any parameters passed to a
function that may trigger GC.
`JS::Handle<T>` exists because creating and destroying a `JS::Rooted<T>`
is not free (though it only costs a few cycles).
Thus, it makes more sense to only root the GC thing once and reuse it
through an indirect reference.
Like a reference, a `JS::Handle` is immutable: it can only ever refer to
the `JS::Rooted<T>` that it was created for.

Similarly to `JS::Rooted<T>`, there are typedefs available for the main
types:

| Template class            | Typedef              |
| ------------------------- | -------------------- |
| `JS::Handle<JS::Value>`   | `JS::HandleValue`    |
| `JS::Handle<JSObject*>`   | `JS::HandleObject`   |
| `JS::Handle<JSString*>`   | `JS::HandleString`   |
| `JS::Handle<JSScript*>`   | `JS::HandleScript`   |
| `JS::Handle<jsid>`        | `JS::HandleId`       |
| `JS::Handle<JSFunction*>` | `JS::HandleFunction` |
| `JS::Handle<JS::Symbol*>` | `JS::HandleSymbol`   |

You should use `JS::Handle<T>` for all function parameters taking GC
thing pointers (except out-parameters, which are described below).
For example, instead of:

```c++
JSObject*
someFunction(JSContext* cx, JSObject* obj) {
    // ...
}
```

You should write:

```c++
JSObject*
someFunction(JSContext* cx, JS::HandleObject obj) {
    // ...
}
```

### JS::MutableHandle<T> ###

All GC thing pointers that are used as out-parameters must be wrapped in
a `JS::MutableHandle<T>`.
A `JS::MutableHandle<T>` is a reference to a `JS::Rooted<T>` that,
unlike a normal handle, may modify the underlying `JS::Rooted<T>`.
All `JS::MutableHandle<T>`s are created through an explicit `&` —
address-of operator — on a `JS::Rooted<T>` instance.
`JS::MutableHandle<T>` is exactly like a `JS::Handle<T>` except that it
adds a `.set(T &t)` method and must be created from a `JS::Rooted<T>`
explicitly.

There are typedefs for `JS::MutableHandle<T>`, the same as for the other
templates:

| Template class                   | Typedef                     |
| -------------------------------- | --------------------------- |
| `JS::MutableHandle<JS::Value>`   | `JS::MutableHandleValue`    |
| `JS::MutableHandle<JSObject*>`   | `JS::MutableHandleObject`   |
| `JS::MutableHandle<JSString*>`   | `JS::MutableHandleString`   |
| `JS::MutableHandle<JSScript*>`   | `JS::MutableHandleScript`   |
| `JS::MutableHandle<jsid>`        | `JS::MutableHandleId`       |
| `JS::MutableHandle<JSFunction*>` | `JS::MutableHandleFunction` |
| `JS::MutableHandle<JS::Symbol*>` | `JS::MutableHandleSymbol`   |

`JS::MutableHandle<T>` should be used for all out-parameters, for
example instead of:

```c++
bool
maybeGetValue(JSContext* cx, JS::Value* valueOut) {
    // ...
    if (!wasError)
        *valueOut = resultValue;
    return wasError;
}

void
otherFunction(JSContext* cx) {
    JS::Value value;
    bool success = maybeGetValue(cx, &value);
    // ...
}
```

You should write:

```c++
bool
maybeGetValue(JSContext* cx, JS::MutableHandleValue valueOut) {
    // ...
    if (!wasError)
        valueOut.set(resultValue);
    return wasError;
}

void
otherFunction(JSContext* cx) {
    JS::RootedValue value(cx);
    bool success = maybeGetValue(cx, &value);
    // ...
}
```

### Return values ###

It's ok to return raw pointers!
These do not need to be wrapped in any of rooting classes, but they
should be immediately used to initialize a `JS::Rooted<T>` if there is
any code that could GC before the end of the containing function; a raw
pointer must never be stored on the stack during a GC.

### AutoRooters ###

GC thing pointers that appear as part of a stack-allocated aggregates
(array, structure, class, union) should use `JS::Rooted<T>` when
possible.

There are some situations when using `JS::Rooted<T>` is not possible, or
is undesirable for performance reasons.
To cover these cases, there are various `AutoRooter` classes that can be
used.

Here are the main AutoRooters defined:

| Type                    | AutoRooter class   |
| ----------------------- | ------------------ |
| `JS::Value[]`           | `AutoArrayRooter`  |
| `js::Vector<JS::Value>` | `AutoValueVector`  |
| `js::Vector<jsid>`      | `AutoIdVector`     |
| `js::Vector<JSObject*>` | `AutoObjectVector` |
| `js::Vector<JSScript*>` | `AutoScriptVector` |

If your case is not covered by one of these, it is possible to write
your own by deriving from `JS::CustomAutoRooter` and overriding the
virtual `trace()` method.
The implementation should trace all the GC things contained in the
object by calling `JS::TraceEdge`.

## GC things on the heap ##

### `JS::Heap<T>` ###

GC thing pointers on the heap must be wrapped in a `JS::Heap<T>`.
The only exception to this is if they are added as roots with the
`JS::PersistentRooted` class, but don't do this unless it's really
necessary.
**`JS::Heap<T>` pointers must also continue to be traced in the normal
way**, which is covered below.

`JS::Heap<T>` doesn't require a `JSContext*`, and can be constructed
with or without an initial value parameter.
Like the other template classes, it functions as if it were the GC thing
pointer itself.

One consequence of having different rooting requirements for heap and
stack data is that a single structure containing GC thing pointers
cannot be used on both the stack and the heap.
In this case, separate structures must be created for the stack and the
heap.

There are currently no convenience typedefs for `JS::Heap<T>`.

For example, instead of this:

```c++
struct HeapStruct
{
    JSObject*  mSomeObject;
    JS::Value  mSomeValue;
};
```

You should write:

```c++
struct HeapStruct
{
    JS::Heap<JSObject*>  mSomeObject;
    JS::Heap<JS::Value>  mSomeValue;
};
```

### Tracing ###

#### `js::NativeObject` ####

All GC pointers stored on the heap must be traced.
For regular `js::NativeObject`s, this is normally done by storing them
in slots, which are automatically traced by the GC.

#### Other JSObjects ####

When defining a JSObject subclass that contains `Heap<T>` fields, [set
the trace hook](https://developer.mozilla.org/en-US/docs/Mozilla/Projects/SpiderMonkey/JSAPI_reference/JSTraceOp)
to invoke a function that traces those fields.

#### General Classes/Structures ####

For a regular `struct` or `class`, tracing must be triggered manually.
The usual way is to define a
`void trace(JSTracer* trc, const char* name)` method on the class —
which is already enough be able to create a `JS::Rooted<YourStruct>` on
the stack — and then arrange for it to be called during tracing.
If a pointer to your structure is stored in the private field of a
JSObject, the usual way would be to define a trace hook on the JSObject
(see above) that casts the private pointer to your structure and invokes
`trace()` on it:

```c++
class MyObject : public JSObject {
    ...
    static void trace(JSTracer* trc, JSObject* obj) {
        MyClass* mine = static_cast<MyClass*>(obj->getPrivate());
        mine->trace(trc, "MyClass private field");
    }
};

class MyClass {
    Heap<JSString*> str;
  public:
    void trace(JSTracer* trc, const char* name) {
        JS::TraceEdge(trc, &str, "my string");
    }
};
```

If a pointer to your structure is stored in some other structure, then
its `trace()` method should invoke yours:

```c++
struct MyOwningStruct {
    MyClass* mything;
    void trace(JSTracer* trc, const char* name) {
        if (mything)
            mything->trace(trc, "my thing");
    }
};
```

If the toplevel structure is not stored in a `JSObject`, then how it
gets traced depends on why it should be alive.
The simplest approach is to use `JS::PersistentRooted` (usable on
anything with a trace method with the appropriate signature):

```c++
JS::PersistentRooted<MyOwningStruct> immortalStruct;
```

But note that `JS::PersistentRooted` in a struct or class is a rather
dangerous thing to use — it will keep a GC thing alive, and most GC
things end up keeping their global alive, so if your class/struct is
reachable in any way from that global, then nothing will ever be cleaned
up by the GC.

## Summary ##

- Use `JS::Rooted<T>` typedefs for local variables on the stack.
- Use `JS::Handle<T>` typedefs for function parameters.
- Use `JS::MutableHandle<T>` typedefs for function out-parameters.
- Use an implicit cast from `JS::Rooted<T>` to get a `JS::Handle<T>`.
- Use an explicit address-of-operator on `JS::Rooted<T>` to get a
  `JS::MutableHandle<T>`.
- Return raw pointers from functions.
- Use `JS::Rooted<T>` fields when possible for aggregates, otherwise use
  an AutoRooter.
- Use `JS::Heap<T>` members for heap data. **Note: `Heap<T>` are not
  "rooted": they must be traced!**
- Do not use `JS::Rooted<T>`, `JS::Handle<T>` or `JS::MutableHandle<T>`
  on the heap.
- Do not use `JS::Rooted<T>` for function parameters.
- Use `JS::PersistentRooted<T>` for things that are alive until the
  process exits (or until you manually delete the PersistentRooted for a
  reason not based on GC finalization.)
