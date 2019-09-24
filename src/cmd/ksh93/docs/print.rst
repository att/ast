.. default-role:: code

:index:`print` -- write arguments to standard output
====================================================

Synopsis
--------
| print [flags] [string ...]

Description
-----------
By default, `print` writes each *string* operand to standard output and
appends a newline character.

Unless, the `-r` or `-f` option is specified, each `\\` character in each
*string* operand is processed specially as follows:

.. parsed-literal::

    `\\a`  Alert character.
    `\\b`  Backspace character.
    `\\c`  Terminate output without appending newline.
         The remaining *string* operands are ignored.
    `\\f`  Formfeed character.
    `\\n`  Newline character.
    `\\t`  Tab character.
    `\\v`  Vertical tab character.
    `\\\\`  Backslash character.
    `\\E`  Escape character (ASCII octal 033).
    `\\0*d*`  The 8-bit character whose ASCII code is the
            1, 2, or 3-digit octal number *d*.

If both `-e` and `-r` are specified, the last one specified is the one
that is used.

When the `-f` option is specified and there are more *string* operands than
format specifiers, the format string is reprocessed from the beginning.
If there are fewer *string* operands than format specifiers, then
outputting will end at the first unneeded format specifier.

Flags
-----
:-e: Unless `-f` is specified, process `\\` sequences in each *string*
    operand as described above. This is the default behavior.

:-n: Do not append a new-line character to the output.

:-f *format*: Write the *string* arguments using the format string *format*
    and do not append a new-line.  See `printf` for details on how to specify
    *format*.

:-p: Write to the current co-process instead of standard output.  Obsolete,
    use `-u p` instead.

:-r: Do not process `\\` sequences in each *string* operand as described
    above.

:-s: Write the output as an entry in the shell history file instead of
    standard output.

:-u *fd*: Write to file descriptor number *fd* instead of standard
    output. If *fd* is `p`, the co-process output file descriptor is used.

:-v: Treat each *string* as a variable name and write the value in
    `%B` format.  Cannot be used with `-f`.

:-C: Treat each *string* as a variable name and write the value in
    `%#B` format.  Cannot be used with `-f`.

:-R: **Do not use this flag.** It's behavior is surprising and likely to
    cause problems. The `-R` option will print all subsequent arguments
    and options other than -n. That, however, is a gross simplification of
    its behavior. For example, `print -Ru2` will cause the `-u2` portion
    to be ignored. Similarly `print -R -n -n val` will consume only the
    first `-n` and print `-n val` without a traiing newline.

Exit Status
-----------
:0: Successful completion.
:>0: An error occurred.

See Also
--------
`echo`\(1), `printf`\(1), `read`\(1)
