.. default-role:: code

:index:`trap` -- trap signals and conditions
============================================

Synopsis
--------
| trap [flags] [action condition ...]

Description
-----------
`trap` is a special built-in that defines actions to be taken when
conditions such as receiving a signal occur.  Also, `trap` can be used
to display the current trap settings on standard output.

If *action* is `-`, `trap` resets each *condition* to the default value.
If *action* is an empty string, the shell ignores each of the *condition*s
if they arise. Otherwise, the argument *action* will be read and executed
by the shell as if it were processed by `eval`\(1) when one of the
corresponding conditions arise.  The action of the trap will override any
previous action associated with each specified *condition*.  The value of
`$?` is not altered by the trap execution.

*condition* can be the name or number of a signal, or one of the following:

:EXIT?This trap is executed when the shell exits.  If defined within a
   function defined with the `function` reserved word, the trap is executed
   in the caller's environment when the function returns and the trap action
   is restored to the value it had when it called the function.

:0: Same as `EXIT`.

:DEBUG: Executed before each simple command is executed but after the
   arguments are expanded.

:ERR: Executed whenever `set -e` would cause the shell to exit.

:KEYBD: Executed when a key is entered from a terminal device.

Signal names are case insensitive and the `sig` prefix is optional.
Signals that were ignored on entry to a noninteractive shell cannot trapped
or reset although doing so will not report an error.  The use of signal
numbers other than `1`, `2`, `3`, `6`, `9`, `14`, and `15` is not portable.

Although `trap` is a special built-in, specifying a condition that the
shell does not know about causes `trap` to exit with a non-zero exit
status, but does not terminate the invoking shell.

If no *action* or *condition*\s are specified then all the current trap
settings are written to standard output.

Flags
-----
:-a: append the current trap setting to the specified *action*.

:-p: Causes the current traps to be output in a format that can be
   processed as input to the shell to recreate the current traps.

:-l: Output the list of signals and their numbers to standard output.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`kill`\(1), `eval`\(1), `signal`\(3)
