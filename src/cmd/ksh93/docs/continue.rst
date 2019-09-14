.. default-role:: code

:index:`continue` -- continue execution at top of the loop
==========================================================

Synopsis
--------
| continue [n]

Description
-----------
`continue` is a shell special built-in that continues execution at the
top of smallest enclosing enclosing `for`, `select`, `while`, or `until`
loop, if any; or the top of the *n*-th enclosing loop if *n* is specified.

If *n* is given, it must be a positive integer >= 1. If *n* is larger
than the number of enclosing loops, the last enclosing loop will be used.

See Also
--------
`break`\(1)
