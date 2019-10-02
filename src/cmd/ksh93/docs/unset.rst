.. default-role:: code

:index:`unset` -- unset values and attributes of variables and functions
========================================================================

Synopsis
--------
| unset [flags] name...

Description
-----------
For each *name* specified, `unset` unsets the variable, or function
if `-f` is specified, from the current shell execution environment.
Readonly variables cannot be unset.

Flags
-----
:-n: If *name* refers to variable that is a reference, the variable *name*
   will be unset rather than the variable it references.  Otherwise, is is
   equivalent to `-v`.

:-f: *aname* refers to a function name and the shell will unset the
   function definition.

:-v: *name* refers to a variable name and the shell will unset it and
   remove it from the environment.  This is the default behavior.

Exit Status
-----------
:0: All *name*\s were successfully unset.

:>0: One or more *name* operands could not be unset or an error occurred.

See Also
--------
`declare`\(1), `typeset`\(1)
