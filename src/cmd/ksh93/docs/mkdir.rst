.. default-role:: code

:index:`mkdir` -- make directories
==================================

Synopsis
--------
| mkdir [flags] directory ...

Description
-----------
`mkdir` creates one or more directories.  By default, the mode of created
directories is `a=rwx` minus the bits set in the `umask`\(1).

Flags
-----
:-m, --mode=*mode*: Set the mode of created directories to *mode*.  *mode*
   is symbolic or octal mode as in `chmod`\(1).  Relative modes assume an
   initial mode of `a=rwx`.

:-p, --parents: Create any missing intermediate pathname components. For
   each dir operand that does not name an existing directory, effects
   equivalent to those caused by the following command shall occur: \vmkdir -p
   -m $(umask -S),u+wx $(dirname dir) && mkdir [-m mode]] dir\v where the `-m`
   mode option represents that option supplied to the original invocation of
   `mkdir`, if any. Each dir operand that names an existing directory shall
   be ignored without error.

:-v, --verbose: Print a message on the standard error for each created directory.


Exit Status
-----------
:0: All directories created successfully, or the `-p` option was
   specified and all the specified directories now exist.

:>0: An error occurred.

See Also
--------
`chmod`\(1), `rmdir`\(1), `umask`\(1)
