# Garbage collection #

As it runs, JavaScript code implicitly allocates memory for objects,
strings, variables, and so on.
Garbage collection is the process by which the JavaScript engine detects
when those pieces of memory are no longer reachable—that is, they could
not possibly ever be used again—and reclaims the memory.

Garbage collection has two important consequences for JSAPI
applications.
First, the application must be very careful to ensure that any values it
needs are GC-reachable.
The garbage collector is rather eager about its job.
Any object you leave lying around will be destroyed if you don't tell
the JSAPI you're still using it.
For more on this topic, see the [GC Rooting Guide](GC Rooting Guide.md).

Second, the application should take steps to reduce the performance
impact of garbage collection.

## GC performance ##

Overly frequent garbage collection can be a performance issue.
Some applications can reduce the frequency of garbage collection simply
by increasing the initial size of the `JSContext`.

Perhaps the best technique is to perform garbage collection during idle
time, when it is least likely to have any impact on the user.
By default, the JavaScript engine performs garbage collection when it
has no other choice except to grow the process.
This means that garbage collection typically happens when
memory-intensive code is running, perhaps the worst possible time.
An application can trigger garbage collection at a more convenient time
by calling `JS_GC` or `JS_MaybeGC`.
`JS_GC` forces garbage collection.
`JS_MaybeGC` performs garbage collection only if it is likely to reclaim
a worthwhile amount of memory.
