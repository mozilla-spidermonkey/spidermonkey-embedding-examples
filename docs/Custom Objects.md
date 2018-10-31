# Creating and initializing custom objects #

In addition to using the engine's built-in objects, you will create,
initialize, and use your own JS objects.
This is especially true if you are using the JS engine with scripts to
automate your application.
Custom JS objects can provide direct program services, or they can serve
as interfaces to your program's services.
For example, a custom JS object that provides direct service might be
one that handles all of an application's network access, or might serve
as an intermediary broker of database services.
Or a JS object that mirrors data and functions that already exist in the
application may provide an object-oriented interface to C code that is
not otherwise, strictly-speaking, object-oriented itself.
Such a custom object acts as an interface to the application itself,
passing values from the application to the user, and receiving and
processing user input before returning it to the application.
Such an object might also be used to provide access control to the
underlying functions of the application.

There are two ways to create custom objects that the JS engine can use:

- Write a JS script that creates an object, its properties, methods, and
  constructor, and then pass the script to the JS engine at run time.
- Embed code in your application that defines the object's properties
  and methods, call the engine to initialize a new object, and then set
  the object's properties through additional engine calls.
  An advantage of this method is that your application can contain
  native methods that directly manipulate the object embedding.

In either case, if you create an object and then want it to persist in
the run time where it can be used by other scripts, you must root the
object by calling `JS_AddRoot` or `JS_AddNamedRoot`.
Using these functions ensures that the JS engine will keep track of the
objects and clean them up during garbage collection, if appropriate.

## Creating an object from a script ##

One reason to create a custom JS object from a script is when you only
need an object to exist as long as the script that uses it is executing.
To create objects that persist across script calls, you can embed the
object code in your application instead of using a script.

**Note**: You can also use scripts to create persistent objects, too.

To create a custom object using a script:

1. Define and spec the object.
   What is it intended to do?
   What are its data members (properties)?
   What are its methods (functions)?
   Does it require a run time constructor function?
2. Code the JS script that defines and creates the object.
   For example: `function myfun(){ var x = newObject(); . . . }`
   **NOTE**: Object scripting using JavaScript occurs outside the
   context of embedding the JS engine in your applications.
   Embed the appropriate JS engine call(s) in your application to
   compile and execute the script.
   You have two choices: 1.) compile and execute a script with a single
   call to `JS::Evaluate`, or 2.) compile the script once with a call to
   `JS::Compile`, and then execute it repeatedly with individual calls
   to `JS_ExecuteScript`.

An object you create using a script only can be made available only
during the lifetime of the script, or can be created to persist after
the script completes execution.
Ordinarily, once script execution is complete, its objects are
destroyed.
In many cases, this behavior is just what your application needs.
In other cases, however, you will want object persistence across
scripts, or for the lifetime of your application.
In these cases you need to embed object creation code directly in your
application, or you need to tie the object directly to the global object
so that it persists as long as the global object itself persists.

## Custom objects ##

An application can create a custom object without bothering with a
`JSClass`:

1. Implement the getters, setters, and methods for your custom object in
   C++.
   Write a `JSNative` for each getter, setter, and method.
2. Declare a `JSPropertySpec` array containing information about your
   custom object's properties, including getters and setters.
3. Declare a `JSFunctionSpec` array containing information about your
   custom object's methods.
4. Call `JS_NewPlainObject` to create the object.
5. Call `JS_DefineProperties` to define the object's properties.
6. Call `JS_DefineFunctions` to define the object's methods.

`JS_SetProperty` can also be used to create properties on an object.
The properties it creates do not have getters or setters; they are
ordinary JavaScript properties.

## Providing private data for objects ##

Like contexts, you can associate large quantities of data with an object
without having to store the data in the object itself.
Call `JS_SetPrivate` to establish a pointer to private data for the
object, and call `JS_GetPrivate` to retrieve the pointer so that you can
access the data.
Your application is responsible for creating and managing this optional private data.

To create private data and associate it with an object:

- Establish the private data as you would a normal C void pointer
  variable.
- Call `JS_SetPrivate`, specify the object for which to establish
  private data, and specify the pointer to the data.

For example:

```c++
JS_SetPrivate(cx, obj, pdata);
```

To retrieve the data at a later time, call `JS_GetPrivate`, and pass the
object as an argument.
This function returns the pointer to an object's private data:

```c++
pdata = JS_GetPrivate(cx, obj);
```
