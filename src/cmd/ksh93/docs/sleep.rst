.. default-role:: code

:index:`sleep` -- suspend execution for an interval
===================================================

Synopsis
--------
| sleep [flags] [duration]

Description
-----------
`sleep` suspends execution for at least the time specified by *duration*
or until a `SIGALRM` signal is received. The *duration* is a floating
point literal for the number of seconds to sleep. The actual granularity
depends on the underlying system, normally around 1 millisecond. This
can be followed by `s` for seconds, `m` for minutes, `h` for hours, or
`d` for days.

Flags
-----
:-s: Sleep until a signal or a timeout is received. If *duration* is
   omitted or less than or equal to zero then no timeout will be used.

Exit Status
-----------
:0: The execution was successfully suspended for at least *duration* or a
   `SIGALRM` signal was received.

:>0: An error occurred.

See Also
--------
`date`\(1), `time`\(1), `wait`\(1)
