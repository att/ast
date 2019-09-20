.. default-role:: code

:index:`dirname` -- return directory portion of file name
=========================================================

Synopsis
--------
| dirname string

Description
-----------
`dirname` treats *string* as a file name and returns the name of the
directory containing the file name by deleting the last component from
*string*.

If *string* consists solely of `/` characters the output will be a single
`/`. Trailing `/` characters are removed, and if there are no remaining
`/` characters in *string*, the string `.` will be written to standard
output.  Otherwise, all characters following the last `/` are removed.
If the remaining string consists solely of `/` characters, the output
will be as if the original string had consisted solely as `/` characters
as described above.  Otherwise, all trailing slashes are removed and the
output will be this string unless this string is empty.  If empty the
output will be `.`.

Flags
-----
:-f, --file: Print the `$PATH` relative regular file path for *string*.

:-r, --relative: Print the `$PATH` relative readable file path for *string*.

:-x, --executable: Print the `$PATH` relative executable file path for *string*.

Exit Status
-----------
:0: Successful Completion.

:>0: An error occurred.

See Also
--------
basename`\(1), `getconf`\(1), `dirname`\(3), `pathname`\(3)
