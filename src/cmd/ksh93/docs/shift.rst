.. default-role:: code

:index:`shift` -- shift positional parameters
=============================================

Synopsis
--------
| shift [n]

Description
-----------
`shift` is a shell special built-in that shifts the positional parameters
to the left by the number of places defined by *n*, or `1` if *n* is
omitted.  The number of positional parameters remaining will be reduced
by the number of places that are shifted.

If *n* is given, it will be evaluated as an arithmetic expression to
determinate the number of places to shift.  It is an error to shift more
than the number of positional parameters or a negative number of places.

Exit Status
-----------
:0: The positional parameters were successfully shifted.

:>0: An error occurred.

See Also
--------
`set`\(1)
