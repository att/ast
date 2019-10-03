.. default-role:: code

:index:`wait` -- wait for process or job completion
===================================================

Synopsis
--------
| wait [flags] [job...]

Description
-----------
`wait` with no operands, waits until all jobs known to the invoking
shell have terminated.  If one or more *job* operands are specified,
`wait` waits until all of them have completed.

Each *job* can be specified as one of the following:

   :*int*: Refers to a process id (pid).
   :`%`\ *int*: Refers to a job number.
   :`%`\ *str*: Refers to a job whose name begins with *str*.
   :`%?`\ *str*: Refers to a job whose name contains *str*.
   :`%%`: Refers to the current job.
   :`%+`: Refers to the current job.
   :`%-`: Refers to the previous job.

If one ore more *job* operands is a process id or process group id not
known by the current shell environment, `wait` treats each of them as if
it were a process that exited with status 127.


Exit Status
-----------
If *wait* is invoked with one or more *job*\s, and all of them have
terminated or were not known by the invoking shell, the exit status of
`wait` will be that of the last *job*.  Otherwise, it will be one of
the following:

:0: `wait` utility was invoked with no operands and all processes known
   by the invoking process have terminated.

:127: *job* is a process id or process group id that is unknown to the
   current shell environment.

See Also
--------
`jobs`\(1), `ps`\(1)
