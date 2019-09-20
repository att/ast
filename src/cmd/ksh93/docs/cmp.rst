.. default-role:: code

:index:`cmp` -- compare two files
=================================

Synopsis
--------
| cmp [flags] file1 file2 [skip1 [skip2]]

Description
-----------
`cmp` compares two files *file1* and *file2*. `cmp` writes
no output if the files are the same. By default, if the
files differ, the byte and line number at which the first
difference occurred are written to standard output. Bytes
and lines are numbered beginning with 1.

If *skip1* or *skip2* are specified, or the `-i` option
is specified, initial bytes of the corresponding file are
skipped before beginning the compare. The skip values are
in bytes or can have a suffix of `k` for kilobytes or `m`
for megabytes.

If either *file1* or *files2* is `-`, `cmp` uses standard
input starting at the current location.

.. index:: builtin

**This command is not enabled by default.** To enable it run `builtin chmod`.

Flags
-----
:-b, --print-bytes: Print differing bytes as 3 digit octal values.

:-c, --print-chars: Print differing bytes as follows: non-space printable
   characters as themselves; space and control characters as `^` followed
   by a letter of the alphabet; and characters with the high bit set
   as the lower 7 bit character prefixed by `M^` for 7 bit space and
   non-printable characters and `M-` for all other characters. If the 7
   bit character encoding is not ASCII then the characters are converted
   to ASCII to determine *high bit set*, and if set it is cleared and
   converted back to the native encoding. Multibyte characters in the
   current locale are treated as printable characters.

:-d *n*, --differences=*n*: Print at most *differences* differences using
   `--verbose` output format. `--differences=0` is equivalent to
   `--silent`.

:-i *skip1*:*skip2*, --ignore-initial=*skip1*:*skip2*, --skip=*skip1*:*skip2*:
   Skip the the first *skip1* bytes in *file1* and the first *skip2*
   bytes in *file2*. If *skip2* is omitted then *skip1* is used.

:-l, --verbose: Write the decimal byte number and the differing bytes
   (in octal) for each difference.

:-n *n*, --count=*n*, --bytes=*n*: Compare at most *n* bytes.

:-s, --quiet, --silent: Write nothing for differing files; return non-zero
   exit status only.

Exit Status
-----------
:0: The files or portions compared are identical.

:1: The files are different.

:>1: An error occurred.

See Also
--------
`comm`\(1), `diff`\(1), `cat`\(1)
