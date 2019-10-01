.. default-role:: code

:index:`ulimit` -- set or display resource limits
=================================================

Synopsis
--------
| ulimit [flags] [limit]

Description
-----------
`ulimit` sets or displays resource limits.  These limits apply to the
current process and to each child process created after the resource
limit has been set.  If *limit* is specified, the resource limit is set,
otherwise, its current value is displayed on standard output.

Increasing the limit for a resource usually requires special privileges.
Some systems allow you to lower resource limits and later increase them.
These are called soft limits.  Once a hard limit is set the resource can
not be increased.

Different systems allow you to specify different resources and some
restrict how much you can raise the limit of the resource.

The value of *limit* depends on the unit of the resource listed for
each resource.  In addition, *limit*\ te no limit for that resource.

If you do not specify `-H` or `-S`, then `-S` is used for listing and both
`-S` and `-H` are used for setting resources.

If you do not specify any resource, the default is `-f`.

Flags
-----
:-H: A hard limit is set or displayed.

:-S: A soft limit is set or displayed.

:-a: Displays all current resource limits

:-M, --as: The address space limit in Kibytes.

:-c, --core: The core file size in blocks.

:-t, --cpu: The cpu time in seconds.

:-d, --data: The data size in Kibytes.

:-f, --fsize: The file size in blocks.

:-x, --locks: The number of file locks.

:-l, --memlock: The locked address space in Kibytes.

:-q, --msgqueue: The message queue size in Kibytes.

:-e, --nice: The scheduling priority.

:-n, --nofile: The number of open files.

:-u, --nproc: The number of processes.

:-p, --pipe: The pipe buffer size in bytes.

:-m, --rss: The max memory size in Kibytes.

:-r, --rtprio: The max real time priority.

:-b, --sbsize: The socket buffer size in bytes.

:-i, --sigpend: The signal queue size.

:-s, --stack: The stack size in Kibytes.

:-w, --swap: The swap size in Kibytes.

:-T, --threads: The number of threads.

:-v, --vmem: The process size in Kibytes.

Exit Status
-----------
:0: Successful completion.

:>0: A request for a higher limit was rejected or an error occurred.

See Also
--------
`ulimit`\(2), `getrlimit`\(2)
