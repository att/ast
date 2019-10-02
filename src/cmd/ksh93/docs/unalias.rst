.. default-role:: code

:index:`unalias` -- remove alias definitions
============================================

Synopsis
--------
| unalias [flags] name...

Description
-----------
`unalias` removes the definition of each named alias from the current shell execution environment, or all aliases if `-a` is specified.  It will not affect any commands that have already been read and subsequently executed.

Flags
-----
:-a: Causes all alias definitions to be removed.  *name* operands are
   optional and ignored in this case.

Exit Status
-----------
:0: Successful completion.

:>0: `-a` was not specified and one or more *name* operands did not
   have an alias definition, or an error occurred.

See Also
--------
`alias`\(1)
