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

## Design overview ##

SpiderMonkey has a mark-sweep garbage collection (GC) with incremental
marking mode, generational collection, and compaction.
Much of the GC work is performed on helper threads.

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

## Principal data structures ##

### Cell ###

A `Cell` is the unit of memory that is allocated and collected by the
GC, as used externally.
In other words, from the point of view of the rest of the engine, the
job of the GC is to allocate cells and automatically collect them.

Cell is the base class for all classes that are allocated by the GC,
e.g., JSObject.

### Allocation Kind ###

Cells are classified by allocation kind.
The allocation kind determines the size of the object and the
finalization behavior.
Allocation kind is defined by `enum AllocKind`.

Arenas always hold objects of the same allocation kind.
Thus, an arena holds objects all of the same size and finalization
behavior.

### Compartments ###

The JS heap is partitioned into compartments.
Some key properties of compartments are:

- Every cell (JS heap object) belongs to at most one compartment.
  (Some cells are shared across all compartments in a Zone.)
- An object may not hold a direct pointer to an object in another
  compartment.
  Instead, it must store a wrapper for the other object.
  This allows compartments to be used for security checks: objects in
  the same compartment have the same access requirements, so no checks
  are needed, but checks may be done when traversing cross-compartment
  wrappers.
- The engine may GC a single compartment without GCing the others.
  The engine can also GC a set of compartments without GCing the rest.
  Cross-compartment wrappers are used as roots for single- and
  multi-compartment GCs.

Compartments are a fundamental cross-cutting concept in SpiderMonkey,
though anything related to memory is more concerned with Zones,
especially GC.

### Zone ###

A zone is a collection of compartments.
Zones mainly serve as boundaries for GCs: the garbage collector collects
at the level of a zone, not an individual compartment.
Unlike compartments, zones have no special security properties.
Some properties of zones are:

- Every compartment belongs to exactly one zone.
- Every JS heap object belongs to exactly one zone.
- Objects from the same zone but different compartments can share an
  arena.
- Objects from different zones cannot be stored in the same arena.
- A zone remains alive as long as any JS heap objects in the zone are
  alive.

### Chunk ###

A Chunk is the largest internal unit of memory allocation.

A chunk is 1MB and contains Arenas, padding, the mark bitmap (ChunkBitmap), a bitmap of decommitted arenas, and a chunk header (ChunkInfo).

The ChunkInfo contains a list of unallocated Arenas, starting at
`ChunkInfo::freeArenasHead` and linked through `ArenaHeader::next`.
The ChunkInfo also contains some basic stats, such as the number of free
arenas.

### Arena ###

An Arena is an internal unit of memory allocation.

An Arena is one page (4096 bytes on almost all platforms) and contains
an ArenaHeader, a few pad bytes, and then an array of aligned elements.
All elements in an Arena have the same allocation kind and size.

Every Arena maintains a list of free spans, starting at
`ArenaHeader::firstFreeSpanOffets`.
The last cell in each free span (except the last one) holds a `FreeSpan`
for the next free span.

### Free Span ###

`struct FreeSpan` represents a contiguous sequence of free cells
`[first,last]` within an Arena.
FreeSpan contains functions to allocate from the free span.

### Mark Bitmap ###

The mark bitmap is represented by `ChunkBitmap`.

The mark bitmap has two bits per GC cell, so that it can represent both
regular liveness marking ("marked black") as well as reachability from
cycle-collected objects ("marked gray").
Mark bits are allocated based on the minimum cell size, thus an object
comprised of larger cells consumes multiple bits in the bitmap (but only
uses two of them.)

## Incremental marking ##

Incremental marking means being able to do a bit of marking work, then
let the mutator (JavaScript program) run a bit, then do another bit of
marking work.
Thus, instead of one long pause for marking, the GC does a series of
short pauses.
The pauses can be set to be 10 ms or less.

There is always a possibility that a long pause will be required.
If the allocation rate is high during incremental GC, the engine may run
out of memory before finishing the incremental GC.
If so, the engine must immediately restart a full, non-incremental GC in
order to reclaim some memory and continue execution.

### Incremental write barrier ###

Incremental GC requires a write barrier for correctness.
The basic problem is as follows.
Say object A is marked black and contains a pointer field.
Let object B be marked white.
Now let the incremental slice stop, so the mutator resumes.
If the mutator stores B into A, so that A contains a pointer to B, and
deletes all existing pointers to B, then:

- B is live, because A is marked black and contains a pointer to B.
- B will not be marked, because B is only reachable through A and we are
  all done with A, because A is marked black.
- Thus, B is live but will be collected.

The write barrier is a piece of code that runs just before a pointer
store occurs and records just enough information to make sure that live
objects don't get collected.

### The SpiderMonkey incremental write barrier ###

SpiderMonkey uses a common incremental write barrier called a
**snapshot-at-the-beginning allocate-black barrier**.

To understand how this barrier works, first assume that we're going to
do an incremental GC during which no new objects are allocated, just to
keep things simple.
How do we make sure the GC doesn't collect any live objects?
One way would be to make sure that every object that was live at the
beginning of the incremental GC gets marked.
(This means if an object has all references to it dropped during this
incremental GC, it will be collected on the next incremental GC.)
This is called **snapshot-at-the-beginning** because it is conceptually
equivalent to taking a snapshot of live objects at the beginning of the incremental GC and marking all those objects.
No such snapshot is physically taken — doing so would require a full nonincremental mark!

The implementation of the snapshot-at-the-beginning barrier is as
follows.
The barrier fires whenever the mutator is about to overwrite a location
that holds a GC pointer.
The barrier marks the pointed-to object black, and pushes any outgoing
edges onto the mark stack.
The key observation is that the only way an object can get 'lost' and
not marked is if all pointers to the object are overwritten.
Thus, if whenever we're about to overwrite a pointer to an object we
mark the object black first, then no objects can get 'lost'.

Now consider allocations.
A newly allocated object wasn't present at the beginning of the GC, so
the snapshot-at-the-beginning barrier doesn't do it any good.
But we do need to make sure the object doesn't get collected if it's
live.
This is easily done by marking new objects immediately upon allocation
during an incremental GC, thus the name **allocate-black**.

### The SpiderMonkey incremental read barrier ###

The textbook version of incremental GC has only a write barrier.
SpiderMonkey also needs a read barrier for weak references.
This is because weak references have a similar problem as is solved by
the incremental write barrier above, but cannot use the same solution.

By definition, a weak reference should not hold a target live by itself.
That means that we can't trace weak references when we encounter them
during GC graph traversal.
Instead, we append them to a list.
After marking everything live we scan through the list to null out
references to garbage.

But this also means that pre-barriers are inadequate to prevent
incremental GC from missing edges due to graph mutation.
Live cells will be marked either because they're reachable or because
they were marked by a pre-barrier before being removed from the graph.
But reachable weak references aren't traced!
So you could read one out and write it behind the frontier and no pre-barriered removal will mark it (it's not being removed, just read).
So we barrier the read instead; if you ever read a weak reference in the
middle of an ongoing incremental GC, then the target of the weak ref
will be marked.

### Implementation details ###

Write barriers have a runtime cost, so SpiderMonkey tries to skip them
when an incremental GC cycle is not active.
Each Zone has a flag `needsIncrementalBarrier()` that indicates whether
barriers are required.

For each type `T` such that fields of type `T*` need a write barrier,
there is a function `T::writeBarrierPre(old)`.
For example, fields of type `JSObject*` need a write barrier, so there
is a function `JSObject::writeBarrierPre(JSObject* old)`.
If `zone->needsIncrementalBarrier()`, then `writeBarrierPre()` marks
`old`.
That's it.

The class `Heap<T>` is provided to make it easy to invoke write
barriers.
A `Heap<T>` encapsulates a `T` and invokes the write barrier whenever
assigned to.
Thus, object fields of GC-pointer type should normally be defined as
type `Heap<T>`.
The class `HeapSlot` (and the related class `HeapSlotArray`) is the
same, but for object slots.

Object private fields must be handled specially.
The private field itself is opaque to the engine, but it may point to
things that need to be marked, e.g., an array of JSObject pointers.
In this example, if the private field is overwritten, the JSObject
pointers could be 'lost', so a write barrier must be invoked to mark
them.
If you need to do this in your objects, invoke the JSObject's class
trace hook before the private field is set.

Another detail is that write barriers can be skipped when initializing
fields of newly allocated objects, because no pointer is being
overwritten.

## Dictionary of terms ##

**black**: In common CS terminology, an object is black during the mark
phase if it has been marked and its children are gray (have been queued
for marking).
An object is black after the mark phase if it has been marked. In SpiderMonkey, an object is black if its black mark bit is set.

**gray**: In common CS terminology, an object is gray during the mark
phase if it has been queued for marking.
In SpiderMonkey, an object is queued for marking if it is on the mark
stack and is not black.
Thus, the gray objects from CS literature are not represented
explicitly.
See also: **gray**, below.

**gray**: SpiderMonkey instead uses "gray" to refer to objects that are
not black, but are reachable from "gray roots".
Gray roots are used by the Gecko cycle collector to find cycles that
pass through the JS heap.

**handle**: In our GC, a Handle is a pointer that has elsewhere been
registered by a root.
In other words, it is an updatable pointer to a GC thing (it is
essentially a `Cell**` that the GC knows about.)

**root**: A starting point to the GC graph traversal, a root is known to
be alive for some external reason (one other than being reachable by
some other part of the GC heap.)

**weak pointer**: In common CS terminology, a weak pointer is one that
doesn't keep the pointed-to value live for GC purposes.
Typically, a weak pointer value has a `get()` method that returns a null
pointer if the object has been GC'd.
In SpiderMonkey, a weak pointer is a pointer to an object that can be
GC'd that is not marked through.
Thus, there is no `get()` method and no protection against the
pointed-to value getting GC'd — the programmer must ensure this
mechanism themselves.

**white**: In common CS terminology, an object is white during the mark
phase if it has not been seen yet.
An object is white after the mark phase if it has not been marked.
In SpiderMonkey, an object is white if it is not gray or black; i.e., it
is not marked and it is not on the mark stack.
