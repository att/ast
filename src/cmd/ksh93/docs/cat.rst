.. default-role:: code

:index:`cat` -- concatenate files
=================================

Synopsis
--------
| cat [flags] [file ...]

Description
-----------
`cat` copies each *file* in sequence to the standard output. If no *file*
is given, or if the *file* is `-`, `cat` copies from standard input
starting at the current location.

.. index:: builtin

**This command is not enabled by default.** To enable it run `builtin cat`.

Flags
-----
:-b, --number-nonblank: Number lines as with `-n` but omit line numbers
    from blank lines.

:-d, --dos-input: Input files are opened in *text* mode which removes
    carriage returns in front of new-lines on some systems.

:-e: Equivalent to `-vE`.

:-n, --number: Causes a line number to be inserted at the beginning of
    each line.

:-s: Equivalent to `-S` for *att* universe and `-B` otherwise.

:-t: Equivalent to `-vT`.

:-u, --unbuffer: The output is not delayed by buffering.

:-v, --show-nonprinting, --print-chars: Print characters as follows:
    space and printable characters as themselves; control characters as
    `^` followed by a letter of the alphabet; and characters with the
    high bit set as the lower 7 bit character prefixed by `M^` for 7 bit
    non-printable characters and `M-` for all other characters. If the 7
    bit character encoding is not ASCII then the characters are converted
    to ASCII to determine *high bit set*, and if set it is cleared and
    converted back to the native encoding. Multibyte characters in the
    current locale are treated as printable characters.

:-A, --show-all: Equivalent to `-vET`.

:-B, --squeeze-blank: Multiple adjacent new-line characters are replace
    by one new-line.

:-D, --dos-output: Output files are opened in *text* mode which inserts
    carriage returns in front of new-lines on some systems.

:-E, --show-ends: Causes a `$` to be inserted before each new-line.

:-S, --silent: `cat` is silent about non-existent files.

:-T, --show-blank: Causes tabs to be copied as `^I` and formfeeds as `^L`.

See Also
--------
`cp`\(1), `getconf`\(1), `pr`\(1)
