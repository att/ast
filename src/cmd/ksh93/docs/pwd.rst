.. default-role:: code

:index:`pwd` -- write working directory name
============================================

Synopsis
--------
| pwd [flags]

Description
-----------
`pwd` writes an absolute pathname of the current working directory to
standard output.  An absolute pathname is a pathname that begins with
`/` that does not contains any `.` or `..` components.

If both `-L` and `-P` are specified, the last one specified will be used.
If neither `-P` or `-L` is specified the default behavior is as if `-L`
was specified.

Flags
-----
:-L: The absolute pathname may contains symbolic link components.  This is
   the default.

:-P: The absolute pathname will not contain any symbolic link components.

:-f *fd*: Print the directory name for the open directory file descriptor
   *fd*. Cannot be combined with other options.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`cd`\(1), `getconf`\(1), `readlink`\(1), `realpath`\(1)
