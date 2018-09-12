# ksh x.y.z (version TBD, this is a work in progress)

This is meant to document changes since the AST code (including the `ksh`
program) was open-sourced that will be in the next stable release based
on the original AST code. The next stable version will be treated as a
major release for several reasons. Not least of which is changing the
build tool chain from the legacy Nmake system to Meson. Legacy changes
can be found in the various `RELEASE` files.

Starting in June 2017 maintenance of the Korn shell (`ksh`) resumed with
the merging of some fixes from Red Hat by Siteshwar Vashisht after he was
granted commit privilege. In October 2017 Kurtis Rader noticed that `ksh`
had been open sourced and started contributing changes. This document
was subsequently created to document the work being done.

## Deprecations

None at this time.

## Notable non-backward compatible changes

- Vi raw mode is now the default and cannot be disabled (issue #864).
- Support for the `universe` command has been removed (issue #793).
- Support for building on systems using EBCDIC has been removed (issue #742).
- Support for the `LC_OPTIONS` env var has been removed (issue #579).
- `case "[0-9]" in [0-9]) echo match;; esac` has stopped matching. When a case
  statement doesn't match a pattern, it no longer tries to use the pattern as
  a literal string (issue #476).
- echo builtin now interprets escape sequences and parses command line options
  on all platforms. (issue #370)
- Support for the UWIN environment has been removed (issue #284).
- The experimental `SHOPT_FIXEDARRAY` feature has been removed. It is known to
  have been enabled in some Fedora and RHEL builds. But because it was
  experimental and undocumented this should not break any existing users of
  `ksh` built with this feature enabled. (issue #234)
- <> operator now redirects standard input by default (issue #75).
- Support for the build time SHOPT_ACCTFILE symbol and code has been removed
  (issue #210).
- Versioning scheme has been changed to use semantic version numbers (issue #335).
- Undocumented and broken `alarm` builtin has been removed (issue #646).

## Notable fixes and improvements

- The project now passes its unit tests when built with malloc debugging
  enabled (i.e., `meson test --setup=malloc`).
- Changes to the project are now validated by running unit tests on the Travis
  continuous integration system.
- The ksh source now builds on BSD based systems such as macOS and FreeBSD.
- The ksh source now builds on Cygwin; albeit with many unit test failures.
- The legacy Nmake build system has been replaced by Meson. This improves the
  build time by roughly an order of magnitude (issue #42).

## Other significant changes

- The AST locale subsystem has been replaced by the platform's implementation
  (issue #579).
- The AST Vmalloc subsystem has been removed. The project now uses the system
  malloc. (issue #396)
- Operations that depend on the simulated 3DFS behavior found in release
  ksh93v are no longer supported. That behavior does not work in ksh93u (the
  most recent stable release found in all distros) unless the 3D file system
  is actually present. This includes operations such as this:
  `exec {fd}</dev; cd ~{fd}` (issue #510).
- The math `scalb` function has been removed since it was marked obsolete by
  POSIX in 2001 and is no longer found in some libc implementations.
- Code hidden behind the SHOPT_ACCT and SHOPT_ACCTFILE build time symbols
  has been removed. The features are not enabled by default and I am not aware
  of any distro which enables them. Furthermore, they are huge security holes
  and should never be enabled.
- The suid_exec program has been removed (issue #366).
- The code has been restyled according to new guidelines (issue #125).
- Many features which used to be optionally included at build time are now
  unconditionally included (e.g., the code protected by `SHOPT_MULTIBYTE`,
  `SHOPT_COMPLETE`, `SHOPT_BRACEPAT`, `SHOPT_RAWONLY`, `SHOPT_STATS,
  `SHOPT_OPTIMIZE`, `SHOPT_SUID_EXEC`, `SHOPT_FILESCAN`, `SHOPT_POLL,
  `SHOPT_AUDIT`, and `SHOPT_SYSRC`).
- Unit tests can now be run under `valgrind` to help detect more bugs.
- Any code not needed to build and run `ksh` has been removed from the master
  branch.
- Fixes backported from OpenSuse (issue #377).
- Fixes backported from Red Hat Fedora and Enterprise Linux (RHEL) distros
  (issue #172).
- Fixes backported from Solaris (issue #122).
