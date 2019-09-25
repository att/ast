.. default-role:: code

:index:`return` -- return from a function or dot script
=======================================================

Synopsis
--------
| return [n]

Description
-----------
`return` is a shell special built-in that causes the function or dot
script that invokes it to exit.  If `return` is invoked outside of a
function or dot script it is equivalent to `exit`.

If `return` is invoked inside a function defined with the `function`
reserved word syntax, then any `EXIT` trap set within the then function
will be invoked in the context of the caller before the function returns.

If *n* is given it will be used to set the exit status.

Exit Status
-----------
If *n* is specified, the exit status is the least significant eight bits
of the value of *n*.  Otherwise, the exit status is the exit status of
preceding command.

See Also
--------
`break`\(1), `exit`\(1)
