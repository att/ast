.. default-role:: code

:index:`whence` -- locate a command and describe its type
=========================================================

Synopsis
--------
| whence [flags] name...

Description
-----------
Without `-v`, `whence` writes on standard output an absolute pathname,
if any, corresponding to *name* based on the complete search order that
the shell uses.  If *name* is not found, then no output is produced.

If `-v` is specified, the output will also contain information that
indicates how the given *name* would be interpreted by the shell in
the current execution environment.

Flags
-----
:-p, -P: Do not check to see if *name* is a reserved word, a built-in,
   an alias, or a function.  This turns off the `-v` option.

:-a: Displays all uses for each *name* rather than the first.

:-f: Do not check for functions.

:-t: Output only the type for each command.

:-q: Quiet mode. Returns 0 if all arguments are built-ins, functions,
   or are programs found on the path.

:-v: For each name you specify, the shell displays a line that indicates
   if that name is one of the following:

   * Reserved word
   * Alias
   * Built-in
   * Undefined function
   * Function
   * Tracked alias
   * Program

Exit Status
-----------
:0: Each *name* was found by the shell.

:1: One or more *name*\s were not found by the shell.

:>1: An error occurred.

See Also
--------
`command`\(1)
