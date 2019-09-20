.. default-role:: code

:index:`echo` -- output a line of text
======================================

Synopsis
--------
| echo [-n] [-e] [arg...]

Description
-----------
`echo` builtin prints all of its arguments separated by space and
terminated by newline.

You should avoid using this command unless you need compatibility with
non-ksh shells like bash. Use `print` instead. That's because whether or
not the `-e` and `-n` flags are available depends on whether the shell is
in *att* or *bsd* mode as determined by `$PATH`..

Flags
-----
:-e: Enable interpreting escape sequences.
   Only available in the *bsd* universe.
   In the *att* universe this flag is printed verbatim.

:-n: Skip putting a newline character at the end of the output.
   Only available in the *bsd* universe.
   In the *att* universe this flag is printed verbatim.

Exit Status
-----------
:0: Successful completion.

:>0: An error such as an invalid flag occurred.

See Also
--------
`print`\(1), `printf`\(1)
