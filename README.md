# A central lock manager for the TSClient LEGACY Build System

Building packages in parallel on multiple build nodes requires to synchronize
access to common resources like a database, in which the packages are stored
persistently. This is an instance of the distributed critical section problem
(only one worker process is allowed to change a package at one time, and while a
package is being changed, no other process may read it). One approach to it is a
central lock manager (in contrast to a distributed lock manager), which should
be sufficient for the needs of a small distributed build system. When time comes
it may be suitable to change to DLM, however this may increase the system's
complexity.

Moreover the resources to protect, i.e. package objects stored in a database and
parts of them, can be grouped in a hierarchical manner. It would be useful to
employ multiple granularity locking to such a structure to allow writes in one
part of a package (i.e. its licenzing information, if stored in a db) while
other parts (i.e. the contained files) may be accessible to other readers (other
packages being built concurrently which may have to depend on that package).

Therefore, TCLM is a centralized, multiple granularity read-write lock manager.

## A thought on recursive locks
Are recursive locks more powerful than \`normal' locks? I mean can we do more
having recursive locks? On point is clear, they solve the problem of functions
being called from external code and thus having to lock used resources, or by
internal code which already locks the resources. With a \`normal' mutex i.e. this
would result in a deadlock because the called function sould wait for the
calling function to unlock their common resources. Of course this cannot happen
because the unlocking code will never be reached.

However in the called function we could simply check if the lock is held by the
current thread and only acquire and release it afterwards if it's not. In
general we could increment a counter each time we try to lock it when it's
already held by the current thread and decrement it when it should be unlocked.
Only when the counter reaches zero we release the lock. This will provide the
same functionality as a recursive lock does with a normal lock, provided it
allows for querying it's state, which can i.e. be achieved with a thread local
variable that tracks the acquired locks.

Hence recursive locks do not provide more power than a \`normal' lock. In fact
the described algorithm shifts the usual implementation of a recursive mutex, by
a spinlock, into the client code.

With shared locks it is not clear if recursive locks are sensitive at all since
the owner of a lock is not well defined. Say a lock is held shared by the
current thread. Then the thread calls a function which requests an exclusive
lock. Should it be granted the request? It acquired the shared lock because it
wanted to put the protected data on hold that it's not modified. It is not aware
that a function may modify it, because it has a shared lock. On the other hand,
the shared lock can be upgraded to an exclusive lock in case no other thread
claims a shared lock. But should that be done by different *code* having not
acquired the shared lock? - Usually not, because the code that acquried the lock
won't know.

To avoid this dilemma, TCLM does not implement recursive locks, in fact
recursive shared locks may not be uniquely defined at all. Furthermore processes
in the sense of TCLM are objects and implemented as such by the client library.
By wrapping them in other objects one can gain something like process local
storage and therefore implement a recursive lock in the client code (and not the
client library) using the above technique.

Additionally such a spinlock would not require server communication anyway and
hence be implemented in the client library (with the drawback of processes being
bound to on client library instance, however usually that is the case since a
process may stand for a local process or even thread).

## Things that may come one day
I think the current version of TCLM implements the most essential functionality.
Furthermore the additional features have in mind at the moment will probably not
add \`power' in a theoretical sense (what can be done with TCLM compared to i.e.
computational power). However this is a list of some thoughts I have, which I
(or someone else) may implement one day. But we'll see. (At least as long we're
not killed by an asteroid or something haha.)

  * UDP transport for small messages
  * Destroying locks
  * A query operation to check if a Process acquired a Lock (would ease the
    client-side implementation of recursive locks but is problematic as one
    Process could i.e. try to acquire multiple X locks and it is not clear which
    one is granted)

## Regarding licensing
I didn't assign a license yet because I couldn't decide uppon one yet. However
the code is publicly available and no one can stop you from using it ...
