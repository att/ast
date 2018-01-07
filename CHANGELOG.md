# ksh x.y.z (version TBD, this is a work in progress)

Starting in June 2017 maintenance of the Korn shell (`ksh`) resumed with
the merging of some fixes from Red Hat by Siteshwar Vashisht after he was
granted commit privelege. In October 2017 Kurtis Rader noticed that `ksh`
had been open sourced and started contributing changes. This document
was subsequently created to document the work being done. Whether the
next stable version is considered a minor or major release is TBD.

## Deprecations

None at this time.

## Notable non-backward compatible changes

- Support for the UWIN environment has been removed (issue #284).
- The `SHOPT_FIXEDARRAY` feature has been removed. This should not break any
  existing uses of `ksh` since that feature is not believed to have been
  enabled in any public release. It could break anyone who was building from
  the source after its last stable release; version `93u+` (issue #234).

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
  and `SHOPT_SYSRC`).
- Unit tests can now be run under `valgrind` to help detect more bugs.
- Any code not needed to build and run `ksh` has been removed from the master
  branch.
- Fixes backported from Solaris (issue #122).
- Fixes backported from Red Hat Fedora and Enterprise Linux (RHEL) distros
  (issue #172).
