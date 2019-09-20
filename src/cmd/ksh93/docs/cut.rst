.. default-role:: code

:index:`cut` -- cut out selected columns or fields of each line of a file
=========================================================================

Synopsis
--------
| cut [flags] [file ...]

Description
-----------
`cut` bytes, characters, or character-delimited fields from one or more
files, contatenating them on standard output.

The option argument *list* is a comma-separated or blank-separated list
of positive numbers and ranges.  Ranges can be of three forms.  The first
is two positive integers separated by a hyphen (*low*`-`*high*), which
represents all fields from *low* to *high*.  The second is a positive
number preceded by a hyphen (`-`\ *high*), which represents all fields
from field `1` to *high*.  The last is a positive number followed by a
hyphen (*low*`-`), which represents all fields from *low* to the last
field, inclusive.  Elements in the *list* can be repeated, can overlap,
and can appear in any order.  The order of the output is that of the input.

One and only one of `-b`, `-c`, or `-f` must be specified.

If no *file* is given, or if the *file* is `-`, `cut` cuts from
standard input.  The start of the file is defined as the current offset.

Flags
-----
:-b, --bytes=*list*: `cut` based on a list of byte counts.

:-c, --characters=*list*: `cut` based on a list of character counts.

:-d, --delimiter=*delim*: The field character for the `-f`
    option is set to *delim*.  The default is the `tab` character.

:-f, fields=*list*: `cut` based on fields separated by the
    delimiter character specified with the `-d` optiion.

:--split: Split multibyte characters selected by the `-b` option. This is the default.

:-n: Do not split multibyte characters selected by the `-b` option.

:-R, -r, --reclen=*reclen*: If *reclen* > 0, the
    input will be read as fixed length records of length *reclen* when
    used with the `-b` or `-c` option.

:-s, --suppress, --only-delimited: Suppress lines with no delimiter
    characters, when used with the `-f` option.  By default, lines with
    no delimiters will be passsed in untouched.

:-D, --line-delimiter, --output-delimiter=*ldelim*:
    The line delimiter character for the `-f` option is set to *ldelim*.
    The default is the `newline` character. Note that `--line-delimeter`
    is also recognized for compatibility with ksh93u+ and earlier versions.

:-N: Do not output new-lines at end of each record when used with the
    `-b` or `-c` option.

:--newline: Output new-lines at end of each record when used with the
    `-b` or `-c` option. On by default.

Exit Status
-----------
:0: All files processed successfully.

:>0: One or more files failed to open or could not be read.

See Also
--------
`paste`\(1), `grep`\(1)
