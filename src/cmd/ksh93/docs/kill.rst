.. default-role:: code

:index:`kill` -- terminate or signal process
============================================

Synopsis
--------
| kill [flags] {-l | -L} [sig ...]
| kill [flags] -s *signame* job ...
| kill [flags] -n *signum* job ...
| kill [flags] [-\ *signame*] job ...
| kill [flags] [-\ *signum*] job ...

Description
-----------
With the first form in which `-l` or `-L` is not specified, `kill` sends
a signal to one or more processes specified by *job*.  This normally
terminates the processes unless the signal is being caught or ignored.

Each *job* can be specified as one of the following:

   :*int*: Refers to a process id (pid).
   :-*int*: Refers to a process group id (pgrp).
   :`%`\ *int*: Refers to a job number.
   :`%`\ *str*: Refers to a job whose name begins with *str*.
   :`%?`\ *str*: Refers to a job whose name contains *str*.
   :`%%`: Refers to the current job.
   :`%+`: Refers to the current job.
   :`%-`: Refers to the previous job.

If the signal is not specified with either the `-n` or the `-s` option,
the `SIGTERM` signal is used.

If `-l` is specified, and no *arg* is specified, then `kill` writes the
list of signals to standard output.  Otherwise, *arg* can be either a
signal name, or a number representing either a signal number or exit
status for a process that was terminated due to a signal.  If a name is
given the corresponding signal number will be written to standard output.
If a number is given the corresponding signal name will be written to
standard output.

Flags
-----
:-l: List signal names or signal numbers rather than sending signals as
   described above.  The `-n` and `-s` options cannot be specified.

:-q *n*: On systems that support *sigqueue*\(2), send a queued signal with
   message number than can only be as large as a signed integer.  The *job*
   must be specified as a positive number. On systems that do not support
   *sigqueue*\(2), a signal is sent without the message number *n* and
   the signal will not be queued.

:-L: Same as `-l` except that if no argument is specified the signals
   will be listed in menu format as with select compound command.

:-n *signum*: Specify a signal number to send.  Signal numbers are not
   portable across platforms, except for the following:

   .. parsed-literal::

      0  No signal
      1  `HUP`
      2  `INT`
      3  `QUIT`
      6  `ABRT`
      9  `KILL`
      14 `ALRM`
      15 `TERM`

:-s *signame*: Specify a signal name to send.  The signal names are
   derived from their names in C header `<signal.h>` without the `SIG`
   prefix and are case insensitive.  `kill -l` will emit the list of
   signals on the current platform.

Exit Status
-----------
:0: At least one matching process was found for each *job* operand, and the
   specified signal was successfully sent to at least one matching process.

:>0: An error occurred.  A value of `2` is returned when `-q` is used and
   it fails with the error EAGAIN.

See Also
--------
`ps`\(1), `jobs`\(1), `kill`\(2), `signal`\(2), `sigqueue`\(2)
