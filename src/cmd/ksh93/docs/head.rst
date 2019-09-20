.. default-role:: code

:index:`head` -- output beginning portion of one or more files
==============================================================

Synopsis
--------
| head [flags] [file ...]

Description
-----------
`head` copies one or more input files to standard output stopping at a
designated point for each file or to the end of the file whichever comes
first. Copying ends at the point indicated by the options. By default a
header of the form `==> `\ *filename*\ ` <==` is output before all but the
first file but this can be changed with the `-q` and `-v` options.

If no *file* is given, or if the *file* is `-`, `head` copies from standard
input starting at the current location.

The option argument for `-c`, and `-s` can optionally be followed by
one of the following characters to specify a different unit other than
a single byte:

   :b: 512 bytes.
   :k: 1-killobyte.
   :m: 1-megabyte.

Flags
-----
For backwards compatibility, `-`\ *number* is equivalent to `-n` *number*.

:-n, --lines=*n*: Copy *n* lines from each file. The default is 10.

:-c, --bytes=*n*: Copy *n* bytes from each file.

:-q, --quiet, --silent: Never output filename headers.

:-s, --skip=*n*: Skip *n* characters or lines from each file before copying.

:-v, --verbose: Always output filename headers.

Exit Status
-----------
:0: All files copied successfully.

:>0: One or more files did not copy.

See Also
--------
`cat`\(1), `tail`\(1)
