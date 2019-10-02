.. default-role:: code

:index:`umask` -- get or set the file creation mask
===================================================

Synopsis
--------
| umask [flags] [mask]

Description
-----------
`umask` sets the file creation mask of the current shell execution
environment to the value specified by the *mask* operand.  This mask
affects the file permission bits of subsequently created files.  *mask* can
either be an octal number or a symbolic value as described in `chmod`\(1).
If a symbolic value is given, the new file creation mask is the complement
of the result of applying *mask* to the complement of the current file
creation mask.

If *mask* is not specified, `umask` writes the value of the file creation
mask for the current process to standard output.

Flags
-----
:-S: Causes the file creation mask to be written or treated as a symbolic
   value rather than an octal number.

:-p: Write the file creation mask in a format that can be use for re-input.

Exit Status
-----------
:0: The file creation mask was successfully changed, or no *mask* operand
   was supplied.

:>0: An error occurred.

See Also
--------
`chmod`\(1)
