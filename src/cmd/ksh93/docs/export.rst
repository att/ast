.. default-role:: code

:index:`export` -- set export attribute on variables
====================================================

Synopsis
--------
| export [flags] [name[=value]...]

Description
-----------
`export` sets the export attribute on each of the variables specified
by *name* which causes them to be in the environment of subsequently
executed commands.  If `=`\ *value* is specified, the variable *name*
is set to *value*.

If no *name*\s are specified then the names and values of all exported
variables are written to standard output.

`export` is built-in to the shell as a declaration command so that field
splitting and pathname expansion are not performed on the arguments.
Tilde expansion occurs on *value*.

Flags
-----
:-n: Remove the variable from the export list.

:-p: Causes the output to be in the form of `export` commands that can
   be used as input to the shell to recreate the current exports.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`sh`\(1), `declare`\(1), `typeset`\(1)
