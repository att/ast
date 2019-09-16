.. default-role:: code

:index:`exit` -- exit the current shell
=======================================

Synopsis
--------
| exit [n]

Description
-----------
`exit` is shell special built-in that causes the shell that invokes it
to exit.  Before exiting the shell, if the `EXIT` trap is set it will
be invoked.

If *n* is given, it will be used to set the exit status.

Exit Status
-----------
If *n* is specified, the exit status is the least significant eight bits
of the value of *n*.  Otherwise, the exit status is the exit status of
the preceding command.  When invoked inside a trap, the preceding command
means the command that invoked the trap.

See Also
--------
`break`\(1), `return`\(1)
