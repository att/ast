# ksh x.y.z (version TBD, this is a work in progress)

This is meant to document changes since the AST (including the `ksh`
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

- Support for the UWIN environment has been removed (issue #284).
- The experimental `SHOPT_FIXEDARRAY` feature has been removed. It is known to
  have been enabled in some Fedora and RHEL builds. But because it was
  experimental and undocumented this should not break any existing users of
  `ksh` built with this feature enabled. (issue #234)

## Notable fixes and improvements

- Changes to the project are now validated by running unit tests on the Travis
  continuous integration system.
- The ksh source now builds on BSD based systems such as macOS and FreeBSD.
- The ksh source now builds on Cygwin.
- The legacy Nmake build system has been replaced by Meson. This improves the
  build time by roughly an order of magnitude (issue #42).

## Other significant changes

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
