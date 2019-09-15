.. default-role:: code

:index:`eval` -- create a shell command and process it
======================================================

Synopsis
--------
| eval [arg ...]

Description
-----------
`eval` is a shell special built-in command that constructs a command
by concatenating the *arg*s together, separating each with a space.
The resulting string is then taken as input to the shell and evaluated
in the current environment.  Note that command words are expanded twice;
once to construct *arg*, and again when the shell executes the constructed
command.

It is not an error if *arg* is not given.

Exit Status
-----------
If *arg* is not specified, the exit status is `0`.  Otherwise, it is the
exit status of the command defined by the *arg* operands.

See Also
--------
`exec`\(1), `trap`\(1), `.`\(1)

