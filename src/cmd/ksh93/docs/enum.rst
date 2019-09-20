.. default-role:: code

:index:`enum` -- create an enumeration type
===========================================

Synopsis
--------
| enum [flags] *typename*[ `=(` *value* ... `)` ]

Description
-----------
`enum` is a declaration command that creates an enumeration type *typename*
that can only store any one of the values in the indexed array variable
*typename*.

If the list of *value*s is omitted, then *typename* must name an indexed
array variable with at least two elements.

When an enumeration variable is used in an arithmetic expression, its
value is the index into the array that defined it starting from index
0. Enumeration strings can be used in an arithmetic expression when
comparing against an enumeration variable.

The enum `_Bool` exists by default with values `true` and `false`. The
predefined alias `bool` is defined as `_Bool`.

Flags
-----
:-i, --ignorecase: The values are case insensitive.

:-p: Writes the enums to standard output. If *typename* is omitted
    then all `enum`\s are written.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`typeset`\(1).
