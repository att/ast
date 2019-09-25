.. default-role:: code

:index:`readonly` -- set readonly attribute on variables
========================================================

Synopsis
--------
| readonly [flags] [name[=value]...]

Description
-----------
`readonly` sets the readonly attribute on each of the variables specified
by *name* which prevents their values from being changed.  If `=`\ *value*
is specified, the variable *name* is set to *value* before the variable
is made readonly.

Within a type definition, if the value is not specified, then a value
must be specified when creating each instance of the type and the value
is readonly for each instance.

If no *name*\s are specified then the names and values of all readonly
variables are written to standard output.

`readonly` is built-in to the shell as a declaration command so that
field splitting and pathname expansion are not performed on the arguments.
Tilde expansion occurs on *value*.

Flags
-----
:-p: Causes the output to be in a form of `readonly` commands that can
   be used as input to the shell to recreate the current set of readonly
   variables.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`sh`\(1), `declare`\(1), `typeset`\(1)
