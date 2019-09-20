.. default-role:: code

:index:`chmod` -- change the access permissions of files
========================================================

Synopsis
--------
| chmod [flags] mode file ...

Description
-----------
`chmod` changes the permission of each file according to mode, which can
be either a symbolic representation of changes to make, or an octal number
representing the bit pattern for the new permissions.

Symbolic mode strings consist of one or more comma separated list of
operations that can be performed on the mode. Each operation is of the form
*user* *op* *perm* where *user* is zero or more of the following letters:

   :u: User permission bits.
   :g: Group permission bits.
   :o: Other permission bits.
   :a: All permission bits. This is the default if none are specified.

The *perm* portion consists of zero or more of the following letters:

   :r: Read permission.
   :s: Setuid when `u` is selected for *who* and setgid when `g` is selected for *who*.
   :w: Write permission.
   :x: Execute permission for files, search permission for directories.
   :X: Same as `x` except that it is ignored for files that do not already
      have at least one `x` bit set.
   :l: Exclusive lock bit on systems that support it. Group execute must be off.
   :t: Sticky bit on systems that support it.

The *op* portion consists of one or more of the following characters:

   :+: Cause the permission selected to be added to the existing
      permissions. `|` is equivalent to `+`.
   :-: Cause the permission selected to be removed to the existing permissions.
   :=: Cause the permission to be set to the given permissions.
   :&: Cause the permission selected to be *and*\ed with the existing permissions.
   :^: Cause the permission selected to be propagated to more restrictive groups.

Symbolic modes with the *user* portion omitted are subject to `umask`\(2)
settings unless the `=` *op* or the `--ignore-umask` option is specified.

A numeric mode is from one to four octal digits (0-7), derived by adding
up the bits with values 4, 2, and 1. Any omitted digits are assumed
to be leading zeros. The first digit selects the set user ID (4) and
set group ID (2) and save text image (1) attributes. The second digit
selects permissions for the user who owns the file: read (4), write (2),
and execute (1); the third selects permissions for other users in the
file's group, with the same values; and the fourth for other users not
in the file's group, with the same values.

For symbolic links, by default, `chmod` changes the mode on the file
referenced by the symbolic link, not on the symbolic link itself. The `-h`
options can be specified to change the mode of the link. When traversing
directories with `-R`, `chmod` either follows symbolic links or does not
follow symbolic links, based on the options `-H`, `-L`, and `-P`.

When the `-c` or `-v` options are specified, change notifications are
written to standard output using the format, `%s: mode changed to %0.4o
(%s)`, with arguments of the pathname, the numeric mode, and the resulting
permission bits as would be displayed by the `ls` command.

For backwards compatibility, if an invalid option is given that is a valid
symbolic mode specification, `chmod` treats this as a mode specification
rather than as an option specification.

.. index:: builtin

**This command is not enabled by default.** To enable it run `builtin chmod`.

Flags
-----
:-H, --metaphysical: Follow symbolic links for command arguments; otherwise
   don't follow symbolic links when traversing directories.

:-L, --logical, --follow: Follow symbolic links when traversing directories.

:-P, --physical, --nofollow: Don't follow symbolic links when traversing directories.

:-R, --recursive: Change the mode for files in subdirectories recursively.

:-c, --changes: Describe only files whose permission actually change.

:-f, --quiet, --silent: Do not report files whose permissioins fail to change.

:-h, -l, --symlink: Change the mode of symbolic links on systems that
   support `lchmod`\(2). Implies `--physical`.

:-i, --ignore-umask: Ignore the `umask`\(2) value in symbolic mode
   expressions. This is probably how you expect `chmod` to work.

:-n, --show: Show actions but do not change any file modes.

:-F *file*, --reference=\ *file*: Omit the *mode* operand and use the mode of *file* instead.

:-v, --verbose: Describe changed permissions of all files.

Exit Status
-----------
:0: All files changed successfully.

:>0: Unable to change mode of one or more files.

See Also
--------
`chgrp`\(1), `chown`\(1), `lchmod`\(1), `tw`\(1), `getconf`\(1), `ls`\(1), `umask`\(2)
