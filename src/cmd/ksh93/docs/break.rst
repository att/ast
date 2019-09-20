.. default-role:: code

:index:`break` -- break out of loop
===================================

Synopsis
--------
| break [n]

Description
-----------
`fg` is a shell special built-in that exits the smallest enclosing
for, select, while, or until loop, or the n-th enclosing loop if n is
specified. Execution continues at the command following the loop(s).

If `n` is given it must be a positive integer >= 1. If n is larger than
the number of enclosing loops the last enclosing loop will be exited.

Exit Status
-----------
0

See Also
--------
`continue`\(1), `return`\(1)
