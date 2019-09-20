.. default-role:: code

:index:`cd` -- change working directory
=======================================

Synopsis
--------
| cd [directory]
| cd old new

Description
-----------
`cd` changes the current working directory of the current shell environment.

In the first form with one operand, if *directory* begins with `/`, or if
the first component is `.` or `..`, the directory will be changed to this
directory.  If directory is `-`, the directory will be changed to the last
directory visited.  Otherwise, if the `CDPATH` environment variable is set,
`cd` searches for *directory* relative to each directory named in the colon
separated list of directories defined by `CDPATH`.  If `CDPATH` not set,
`cd` changes to the directory specified by *directory*.

In the second form, the first occurrence of the string *old* contained in
the pathname of the present working directory is replaced by the string
*new* and the resulting string is used as the directory to which to change.

When invoked without operands and when the `HOME` environment variable
is set to a nonempty value, the directory named by the `HOME` environment
variable will be used.  If `HOME` is empty or unset, it will use the home
directory for the effective user if possible.  Otherwise `cd` will fail.

When `cd` is successful, the `PWD` environment variable will be set to the
name of an absolute pathname that does not contain any `..` components
corresponding to the new directory.  The environment variable `OLDPWD`
will be set to the previous value of `PWD`.  If the new directory is
found by searching the directories named by `CDPATH`, or if *directory*
is `-`, or if the two operand form is used, the new value of `PWD` will
be written to standard output.

If both `-L` and `-P` are specified, the last one specified will be used.
If neither `-P` or `-L` is specified the default behavior is as if `-L`
was specified.

Flags
-----
:-f *dirfd*: The directory path is relative to the open file descriptor *dirfd*.

:-L: Handle each pathname component `..` in a logical fashion by moving
   up one level by name in the present working directory.

:-P: The present working directory is first converted to an absolute
   pathname that does not contain symbolic link components and symbolic
   name components are expanded in the resulting directory name.

:-@: Change into the hidden attribute directory of *directory* which may
   also be a file. `CDPATH` is ignored. Hidden attribute directories are
   file system and operating system specific.


Exit Status
-----------
:0: Directory successfully changed.

:>0: An error occurred.

See Also
--------
`pwd`\(1)
