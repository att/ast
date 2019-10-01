.. default-role:: code

:index:`test` -- evaluate expression
====================================

Synopsis
--------
| test [expression]

Description
-----------
`test` evaluates expressions and indicates its results based on the exit
status.  Option parsing is not performed so that all arguments, including
`--` are processed as operands.  The evaluation of the expression depends
on the number of operands as follows:

0. Evaluates to false.

1. True if argument is not an empty string.

2. If first operand is `!`, the result is True if the second operand an
   empty string.  Otherwise, it is evaluated as one of the unary expressions
   defined below.  If the unary operator is invalid and the second argument is
   `--`,

then the first argument is processed as an option argument.

3. If first operand is `!`, the result is True if the second and third
   operand evaluated as a unary expression is False.  Otherwise, the three
   operands are evaluaged as one of the binary expressions listed below.

4. If first operand is `!`, the result is True if the next three operands
   are a valid binary expression that is False.

If any *file* is of the form `/dev/fd/`\ *n*, then file descriptor *n*
is checked.

Unary expressions can be one of the following:

:-a *file*: True if *file* exists, obsolete.

:-b *file*: True if *file* exists and is a block special file.

:-c *file*: True if *file* exists and is a character special file.

:-d *file*: True if *file* exists and is a directory.

:-e *file*: True if *file* exists.

:-f *file*: True if *file* exists and is a regular file.

:-g *file*: True if *file* exists and has its set-group-id bit set.

:-h *file*: True if *file* exists and is a symbolic link.

:-k *file*: True if *file* exists and has its sticky bit on.

:-n *string*: True if length of *string* is non-zero.

:-o *option*: True if the shell option *option* is enabled.

:-p *file*: True if *file* exists and is a pipe or fifo.

:-r *file*: True if *file* exists and is readable.

:-s *file*: True if *file* exists and has size > 0.

:-t *fildes*: True if file descriptor number *fildes* is open and is
   associated with a terminal device.

:-u *file*: True if *file* exists and has its set-user-id bit set.

:-v *varname*: True if *varname* is a valid variable name that is set.

:-w *file*: True if *file* exists and is writable.

:-x *file*: True if *file* exists and is executable.  For a directory it
   means that it can be searched.

:-z *string*: True if *string* is a zero length string.

:-G *file*: True if *file* exists and group is the effective group id of
   the current process.

:-L *file*: True if *file* exists and is a symbolic link.

:-N *file*: True if *file* exists and has been modified since it was last read.

:-O *file*: True if *file* exists and owner is the effective user id of
   the current process.

:-R *varname*: True if *varname* is a name reference.

:-S *file*: True if *file* exists and is a socket.

Binary expressions can be one of the following:

:*string1* = *string2*: True if *string1* is equal to *string2*.

:*string1* == *string2*: True if *string1* is equal to *string2*.

:*string1* != *string2*: True if *string1* is not equal to *string2*.

:*num1* -eq *num2*: True if numerical value of *num1* is equal to *num2*.

:*num1* -ne *num2*: True if numerical value of *num1* is not equal to *num2*.

:*num1* -lt *num2*: True if numerical value of *num1* is less than *num2*.

:*num1* -le *num2*: True if numerical value of *num1* is less than or equal to *num2*.

:*num1* -gt *num2*: True if numerical value of *num1* is greater than *num2*.

:*num1* -ge *num2*: True if numerical value of *num1* is greater than or equal to *num2*.

:*file1* -nt *file2*: True if *file1* is newer than *file2* or *file2* does not exist.

:*file1* -ot *file2*: True if *file1* is older than *file2* or *file2* does not exist.

:*file1* -ef *file2*: True if *file1* is another name for *file2*.
   This will be true if *file1* is a hard link or a symbolic link to *file2*.

Exit Status
-----------
:0: Indicates that the specified expression is True.

:1: Indicates that the specified expression is False.

:>1: An error occurred.

See Also
--------
`let`\(1), `expr`\(1)
