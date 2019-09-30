.. default-role:: code

:index:`shcomp` -- compile a shell script
=========================================

Synopsis
--------
| shcomp [flags] [infile [outfile]]

Description
-----------
Unless `-D` is specified, `shcomp` takes a shell script, *infile*, and creates a binary format file, *outfile*, that `ksh` can read and execute with the same effect as the original script.

Since aliases are processed as the script is read, alias definitions whose value requires variable expansion will not work correctly.

If `-D` is specified, all double quoted strings that are preceded by `$` are output.  These are the messages that need to be translated to locale specific versions for internationalization.

If *outfile* is omitted, then the results will be written to standard output.  If *infile* is also omitted, the shell script will be read from standard input.

Flags
-----
:-D, --dictionary: Generate a list of strings that need to be placed in
   a message catalog for internationalization.

:-n, --noexec: Displays warning messages for obsolete or non-conforming constructs.

:-v, --verbose: Displays input from *infile* onto standard error as it reads it.

Exit Status
-----------
:0: Successful completion.

:>0: An error occurred.

See Also
--------
`ksh`\(1)
