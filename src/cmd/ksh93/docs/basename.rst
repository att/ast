.. default-role:: code

:index:`basename` -- strip directory and suffix from filenames
==============================================================

Synopsis
--------
| basename [flags] string [suffix]
| basename [flags] string...

Description
-----------
`basename` removes all leading directory components from the file name
defined by *string*. If the file name defined by *string* has a suffix
that ends in *suffix*, it is removed as well.

If *string* consists solely of **/** characters the output will be a
single **/**. Trailing **/** characters are removed, and if there are
any remaining **/** characters in *string*, all characters up to and
including the last **/** are removed. Finally, if *suffix* is specified,
and is identical the end of *string*, these characters are removed. The
characters not removed from *string* will be written on a single line to
the standard output.

.. index:: builtin

**This command is not enabled by default.** To enable it run `builtin basename`.

Flags
-----
:-a, --all: All operands are treated as *string* and each modified pathname
   is printed on a separate line on the standard output.

:-s, --suffix: All operands are treated as *string* and each modified
   pathname, with *suffix* removed if it exists, is printed on a separate
   line on the standard output.


Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`dirname`\(1), `basename`\(3)
