.. default-role:: code

:index:`uname` -- identify the current system
=============================================

Synopsis
--------
| uname [flags] [name ...]

Description
-----------
By default `uname` writes the operating system name to standard
output. When options are specified, one or more system characteristics
are written to standard output, space separated, on a single line. When
more than one option is specified the output is in the order specified
by the `-A` option below.  Unsupported option values are listed as
*[option]]*. If any unknown options are specified then the local
`/usr/bin/uname` is called.

If any *name* operands are specified then the `sysinfo`\(2) values for
each *name* are listed, separated by space, on one line.  `getconf`\(1), a
pre-existing *standard* interface, provides access to the same information;
vendors should spend more time using standards than inventing them.

Selected information is printed in the same order as the options below.

Flags
-----
:-a, --all: Equivalent to `-snrvmpio`.

:-s, --system, --sysname, --kernel-name: The detailed kernel name. This
   is the default.

:-n, --nodename: The hostname or nodename.

:-r, --release, --kernel-release: The kernel release level.

:-v, --version, --kernel-version: The kernel version level.

:-m, --machine: The name of the hardware type the system is running on.

:-p, --processor: The name of the processor instruction set architecture.

:-i, --implementation, --platform, --hardware-platform: The hardware
   implementation; this is `--host-id` on some systems.

:-o, --operating-system: The generic operating system name.

:-h, --host-id, --id: The host id in hex.

:-d, --domain: The domain name returned by `getdomainname`\(2).

See Also
--------
`hostname`\(1), `uname`\(2)
