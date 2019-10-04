.. default-role:: code

:index:`read` -- read a line from standard input
================================================

Synopsis
--------
| read [flags] [var?prompt] [var ...]

Description
-----------
`read` reads a line from standard input and breaks it into fields using
the characters in value of the `IFS` variable as separators.  The escape
character, `\\`, is used to remove any special meaning for the next
character and for line continuation unless the `-r` option is specified.

If there are more variables than fields, the remaining variables are set
to empty strings.  If there are fewer variables than fields, the leftover
fields and their intervening separators are assigned to the last variable.
If no *var* is specified then the variable `REPLY` is used.

When *var* has the binary attribute and `-n` or `-N` is specified, the
bytes that are read are stored directly into `var`.

If you specify `?`\ *prompt* after the first *var*, then `read` will
display *prompt* on standard error when standard input is a terminal
or pipe.

If an end of file is encountered while reading a line the data is read
and processed but `read` returns with a non-zero exit status.

Flags
-----
:-A: Unset *var* and then create an indexed array containing each field
   in the line starting at index 0.

:-C: Unset *var* and read *var* as a compound variable.  Equivalent to `-aksh`.

:-a: Unset *var* and then create an indexed array containing each field
   in the line starting at index 0.

:-d *delim*: Read until delimiter *delim* instead of to the end of line.

:-p *prompt*: Write *prompt* on each line before reading.  In earlier
   releases `-p` caused the input to come from the current co-process.
   Use `-u p` instead.  For backward compatibility, if there is a
   co-process active and *prompt* is a valid identifier, then *prompt*
   will be treated as an argument and it will read from the co-process
   instead of using *prompt* as the prompt.

:-r: Do not treat `\\` specially when processing the input line.

:-s: Save a copy of the input as an entry in the shell history file.

:-S: Treat the input as if it was saved from a spreadsheet in csv format.

:-u *fd*: Read from file descriptor number *fd* instead of standard
   input. If *fd* is `p`, the co-process input file descriptor is
   used. Defaults to zero.

:-t *timeout*: Specify a timeout *timeout* in seconds when reading from
   a terminal or pipe. The minimum timeout is one. Timeouts less than
   one will be treated as one.

:-n *count*: Read at most *count* characters.  For binary fields *count*
   is the number of bytes.

:-N *count*: Read exactly *ncount* characters.  For binary fields *count*
   is the number of bytes.

:-v: When reading from a terminal the value of the first variable is
   displayed and used as a default value.

Exit Status
-----------
:0: Successful completion.

:>0: End of file was detected or an error occurred.

See Also
--------
`print`\(1), `printf`\(1), `cat`\(1)
