.. default-role:: code

:index:`sync` -- schedule file/file system updates
==================================================

Synopsis
--------
| sync [flags]

Description
-----------
`sync` transfers buffered modifications of file metadata and data to
the storage device for a specific file, a specific filesystem, or all
filesystems.

At minimum `sync` (with no options) should be called before halting the
system. Most systems provide graceful shutdown procedures that include
`sync` -- use them if possible.

Flags
-----
:-f, --sfsync: Calls `sfsync`\(3) to flush all buffered *sfio* stream data.

:-s, --fsync=*fd*: Calls `fsync`\(2) using the open file descriptor *fd*
   to transfer all data associated with *fd* to the storage device. `sync`
   waits until the transfer completes or an error is detected.

:-S, --syncfs=*fd*: Calls `syncfs`\(2) using the open file descriptor
   *fd* to transfer all data for the file system containing the file
   referred to by *fd*. Depending on the native system implementation
   `sync` may return before the data is actually written. Implies
   `--sfsync`.

:-X, --sync, --all: Calls `sync`\(2) to transfer all data for
   all filesystems. Depending on the native system implementation the
   writing, although scheduled, is not necessarily complete upon return from
   `sync`. Since `sync`\(2) has no failure indication, `sync` only fails
   for option/operand syntax errors, or when `sync`\(2) does not return,
   in which case `sync`\(1) also does not return. Implies `--sfsync`. This
   is the default when no options are specified.

See Also
--------
`fsync`\(2), `sync`\(2), `syncfs`\(2), `sfsync`\(3), `shutdown`\(8)
