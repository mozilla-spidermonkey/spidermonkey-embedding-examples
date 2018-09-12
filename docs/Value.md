# JavaScript values #

JavaScript is a dynamically typed language: variables and properties do
not have a type that is fixed at compile time.
How can a statically typed language, like C or C++, in which all
variables have types, interact with JavaScript?
The JSAPI provides a data type, `JS::Value`, which can contain
JavaScript values of any type.
A `JS::Value` can be a number, a string, a boolean value, a reference to
an object (like an `Object`, `Array`, `Date`, or `Function`), or one of
the special values `null` or `undefined`.

For integers and boolean values, a `JS::Value` contains the value
itself.
In other cases, the `JS::Value` is a pointer to an object, string, or
number.

> **Warning:** Like C++ pointers, and unlike JavaScript `var`s, a
> `JS::Value` is **not** automatically initialized to a safe value, and
> **can** become a dangling pointer!
>
> A dangling pointer is a pointer that used to point to a valid object,
> but no longer does because the object no longer exists.
> Using a dangling pointer can crash a C++ program (or worse).
> In the case of `JS::Value`, the JavaScript garbage collector recycles
> objects, strings, and numbers that don't appear to be in use, and a
> `JS::Value` by itself does not protect its referent from the garbage
> collector.
> See _Garbage collection_ below for crucial information on how to use
> `JS::Value`s safely.

`JS::Value` inclues member functions to test the JavaScript data type.
These are `isObject()`, `isNumber()`, `isInt32()`, `isDouble()`,
`isString()`, `isBoolean()`, `isSymbol()`, `isNull()`, and
`isUndefined()`.

If a `JS::Value` contains a `JSObject`, `double`, or `JSString`, you can
cast it to its underlying data type using the `toObject()`,
`toDouble()`, and `toString()` member functions, respectively.
This is useful in some cases where your application or a JSAPI function
requires a variable or argument of a specific data type, rather than a
`JS::Value`.
Similarly, you can create a `JS::Value` wrapping a `JSObject`, `double`,
or `JSString` pointer to a `JS::Value` using
`JS::ObjectValue(JSObject&)`, `JS::DoubleValue(double)`, or
`JS::StringValue(JSString*)`.
