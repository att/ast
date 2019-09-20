.. default-role:: code

:index:`let` -- evaluate arithmetic expressions
===============================================

Synopsis
--------
| let [expr ...]

Description
-----------
`let` evaluates each *expr* in the current shell environment as an
arithmetic expression using ANSI C syntax.  Variables names are shell
variables and they are recursively evaluated as arithmetic expressions
to get numerical values.

`let` has been made obsolete by the `((` ... `))` syntax of `ksh`\(1)
which does not require quoting of the operators to pass them as command
arguments.

Exit Status
-----------
:0: The last *expr* evaluates to a non-zero value.

:>0: The last *expr* evaluates to `0` or an error occurred.

See Also
--------
`expr`\(1), `test`\(1), `ksh`\(1)
