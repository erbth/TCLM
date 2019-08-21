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

Moreover the locks in the ecosystem can be arranged in a tree-like structure,
but with multiple roots. "Smaller" locks, which are more specific in terms of
the exact resources used, can be children of multiple parent locks. For instance
think of a directory on a filesystem. It is a child of the directory it is
contained in and of maybe a package, when it represents scratch space for
building the package. A process can lock the entire package with all its
resources, including the directory, while another process may lock the entire fs
tree, perhaps to create a backup. When each "resource", object is assigned a
lock the lock-graph's topology would be the same as the resources'.

However the resulting graph will be directed, cyclic free and "connected" (each
node has at least one incoming or outgoing edge) but not stronlgy connected.
Hence it is not quite a tree but almost.

The lock manager supports "forests" of such tree-like objects, too.
