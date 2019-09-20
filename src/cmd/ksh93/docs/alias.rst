.. default-role:: code

:index:`alias` -- define or display aliases
===========================================

Synopsis
--------
| alias [flags] [name[=value]...]

Description
-----------
`alias` creates or redefines alias definitions or writes the existing
alias definitions to standard output.  An alias definition provides a
string value that will replace a command name when the command is read.
Alias names can contain any printable character which is not special
to the shell.  If an alias value ends in a space or tab, then the word
following the command name the alias replaces is also checked to see
whether it is an alias.

If no *name*'s are specified then the names and values of all aliases
are written to standard output.  Otherwise, for each *name* that is
specified, and **=**\ *value* is not specified, the current value of the
alias corresponding to *name* is written to standard output.  If **=**\
*value* is specified, the alias *name* will be created or redefined.

`alias` is built-in to the shell as a declaration command so that field
splitting and pathname expansion are not performed on the arguments.
Tilde expansion occurs on *value*.  An alias definition only affects
scripts read by the current shell environment.  It does not effect scripts
run by this shell.

Flags
-----
:-p: Causes the output to be in the form of alias commands that can be used
   as input to the shell to recreate the current aliases.

.. index:: pair: alias; PATH

:-t: Used for :index:`tracked aliases`.  These are aliases that connect
   a command name to the pathname of the command and are reset when the
   `PATH` variable is unset.  The tracked aliases feature is now obsolete.

:-x: Ignored, this option is obsolete.

Exit Status
-----------
:0: Successful completion.

:>0: One or more *name* operands did not have an alias definition, or an
   error occurred.

See Also
--------
`sh`\(1), `unalias`\(1)
