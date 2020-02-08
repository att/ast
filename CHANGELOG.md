# This branch is not supported

This branch contains the **ksh2020** version of the AST tools, from January
2020, which primarily written and maintained by @krader1961 and @siteshwar.

This branch is not supported or maintained by AT&amp;T; the repo at
https://github.com/att/ast only officially hosts ksh93u+ and the experimental
ksh93v- branch.

See [#1464](https://github.com/att/ast/issues/1464) and
[#1466](https://github.com/att/ast/issues/1466) for discussion and history of
ksh2020 leaving the github/att organization.

Please search for forks of this repo if you are looking for ksh2020.


#ksh 2020.1.0 (this is a work in progress)

## Deprecations

## Notable non-backward compatible changes

- You can no longer use arbitrary expressions in imported numeric env vars;
  only integer literals are allowed.
- The `login` and `newgrp` special builtins have been removed. Use
  `exec login` and `exec newgrp` instead (#1348).

## Notable fixes and improvements

- The default build type is now "minsize" since that dramatically improves the
  performance of ksh. You can still request a debug build via `meson
  --buildtype=debug` (issue #1449).
- Fix `history` command behavior when the *~/.sh_history* file has
  specific content (issue #1432).
- A ${.sh.install_prefix}/config.ksh file will be sourced if it exists. It is
  the first config file loaded by every ksh instance. It is loaded regardless
  of whether the shell is interactive, login, or neither.
- A .sh.install_prefix var has been introduced to reflect the `meson -prefix=`
  value. This will be used to establish the location of supporting files such
  as man pages and autoloaded functions.
- Cd related functions (e.g., cd, pushd, popd, nextd, prevd, mcd, dirs) are
  automatically enabled. The code is based on the functions that used to be
  found in src/cmd/ksh93/fun. In practice they were never used because most
  distros never installed those files, and if installed no one knew about them.

## Other significant changes

# ksh 2020.0.0

This documents changes since the AST code, which includes the `ksh` program,
was moved to Github. That is, the ksh93u+ source. The next stable version will
be treated as a major release for several reasons. Not least of which is
changing the build tool chain from the legacy Nmake system to Meson and
replacing the AST Vmalloc subsystem with the platform Malloc subsystem. Legacy
changes can be found in the various `RELEASE` files.

Starting in June 2017 maintenance of the Korn shell (`ksh`) resumed with
the merging of some fixes from Red Hat by Siteshwar Vashisht after he was
granted commit privilege. In October 2017 Kurtis Rader noticed that `ksh`
had been open sourced and started contributing changes. This document
was subsequently created to document the work being done.

## Deprecations

- Some libc/libm math functions not suitable for use by a ksh script (e.g.,
  `j0()`) will likely be removed in the near future (issue #1346 and #88).

## Notable non-backward compatible changes

- The nonportable, unusable, `fpclassify` math function has been removed
  (issue #1346).
- The broken math functions `nextforward` and `nexttoward` have been removed
  (issue #1346).
- Support for binary plugins written for ksh93u+ or earlier releases has been
  dropped (issue #983).
- Support for coshell has been removed (issue #619).
- The `universe` command has been removed (issue #793).
- The `getconf` command has been removed (issue #1118).
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

- `declare` has been added as an alias for `typeset` (issue #220).
- `local` has been added as a constrained alias for `typeset` when used inside
  a function (issue #220).
- Mention of the `getconf` builtin has been removed from the main ksh man
  page. That command has never been enabled by default and is now deprecated
  in favor of the platform command of the same name (issue #1118).
- The `test` command no longer silently fails all uses of the `=~` operator.
  Instead an error is printed suggesting the use of `[[...]]` (issue #1152).
- Doing `[ -t1 ]` inside a command substitution behaves correctly
  (issue #1079).
- The project now passes its unit tests when built with malloc debugging
  enabled (i.e., `meson test --setup=malloc`) or with ASAN enabled.
- Changes to the project are now validated by running unit tests on the Travis
  continuous integration system.
- The ksh source now builds on BSD based systems such as macOS and FreeBSD.
- The ksh source now builds on Cygwin; albeit with many unit test failures.
- The legacy Nmake build system has been replaced by Meson. This improves the
  build time by roughly an order of magnitude (issue #42).
- The `times` command is now a builtin that conforms to POSIX rather than an
  alias for the `time` command (issue #16).
- The `time` command now has millisecond resolution if the platform provides
  `getrusage()` and its time values have millisecond or better resolution.

## Other significant changes

- Using a bare tilde (e.g., `cd ~`) now works correctly if `$HOME` is not set
  (issue #1391).
- You can no longer run an external command with stdin, stdout, or stderr
  closed. If you attempt to do so (e.g., `a_cmd <&-`) it will be opened on
  /dev/null in the child process (issue #1117).
- Vi raw mode is now the default and cannot be disabled. Note that this was
  true at least as far back as ksh93u+. The difference is that now it's no
  longer even theoretically possible to even build with that feature disabled.
  (issue #864).
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
- Unit tests can now be run under Valgrind and ASAN to help detect more bugs.
- Any code not needed to build and run `ksh` has been removed from the master
  branch.
- Fixes backported from OpenSuse (issue #377).
- Fixes backported from Red Hat Fedora and Enterprise Linux (RHEL) distros
  (issue #172).
- Fixes backported from Solaris (issue #122).

## Other significant changes taken from ksh93v-:

Last stable release was `ksh93u+`. ksh x.y.z is based on `ksh93v-` which was last beta
release that came out from AT&T Bell Labs. It had number of notable changes that are
listed below. Entries are directly taken from `src/cmd/ksh93/RELEASE` file (with minor
spelling fixes):

- 14-12-02  The requirement that unquoted { and } must match inside string with
	  ${name op string} has been removed (at least for now).
- 14-09-24 +When listing jobs, the shell now shows the directory that the job
	  was started from when it was not started from the current working
	  directory.
- 14-07-11 +Added -t flag and -P flag to whence and type for bash compatibility.
- 14-07-11 +Added -p flag to alias to output aliases for re-input.
- 14-06-30 +The variable COMP_WORDBREAKS for programmable completion.
- 14-06-25 +The -D and -E options have been added to the complete builtin.
- 14-06-19 +The -l flag to trap and the -p flag to umask were added as in bash.
- 14-06-16 +Added parameter expansion operator ${$parameter} for variables and
	  positional parameters.
- 14-06-06  Added -a option to read which is equivalent to -A.
- 14-06-05 +Added -n option to builtin to disable builtins.
- 14-05-25 +Replaced the -p option for read with -p prompt.  For backward
	  compatibility, if a coprocess is running and prompt begins with -
	  or is a valid variable name, -p causes the read from a pipe.
- 14-05-25 +Modified the -u option for read and print so that it accepts the
	  option argument p to indicate the coprocess file descriptor.
- 14-01-10  [[...]] now supports hexadecimal constants with arithmetic operators.
- 13-12-05  If cd is invoked with no arguments and HOME is unset, it attempts to
	  find the home directory and use that.  Otherwise an error occurs.
- 13-11-14 +Added -f fd option to pwd to display the directory corresponding to
	  file descriptor fd.
- 13-10-08  The shell arithmetic now recognized suffices f,F, l, and L for
	  floating point constants.
- 13-10-07  The shell now prints an error message on standard error when a
	  job specified with %job does not exist.
- 13-09-13 +The signal .sh.value variable is now a compound variable with the name
	  value.q for sending a signal with -q and value.Q for sending a value
	  with -Q.
- 13-09-10 +A -Q option was added to kill to pass integers as large as pointers.
	  The -q option now only accepts integers as large as typeset -i.
- 13-09-09  Qualified print format "%([no]unicode)q" added to prefer \u[...]
	  over \w[...] and override LC_OPTIONS=unicode.
- 13-09-04 +\w[hex] locale-specific code point literals have been added.
- 13-09-04 +The float(f) math function was added.
- 13-09-04  kill -q can now pass numbers as large as typeset -li and
	  .sh.sig.value is typeset -i rather than a compound variable.
- 13-09-04  kill -q yields the processor and returns 2 when siqueue fails with
	  EAGAIN and yield.
- 13-08-26 +Added 12 math constants such as E, PI, and SQRT2.
- 13-08-19 +The variable .sh.pwdfd which expands to the file descriptor number
	  corresponding to $PWD has been added.
- 13-08-11 +namespace was modified so that namespace names can be a compound
	  variable rather than just an identifier and namespace are no
	  longer nested.
- 13-08-07  typeset -p (and print -v) now display the short attribute for
	  typeset -sF and typeset -sE.
- 13-08-06 +You can now use the redirection <& {n} which is the same as <& $n.
- 13-07-24  The _ variable is now set as a reference to the type inside
	  discipline functions for non-type variables in the type.
- 13-07-22  The .sh.sig variable has been modified to treat .sh.value as a
	  compound variable containing int and ptr.
- 13-07-18  Assignments of the type name=(...) to array variables now preserve
	  the array type and the variable type if any.
- 13-07-18  If a PATH ends in ., and you are in the current directory and this
	  directory is in FPATH, ksh now treats this as a function directory.
- 13-07-17  ${!.sh.sig@]} now expands to all the .sh.sig. variables.
- 13-07-08  When using kill -q to send a signal, a CONT signal is not sent to
	  wake the process if it is sleeping after sending the signal.
- 13-06-21  A -f nn option has been added to cd to change to a directory
	  relative to a file descriptor of an open directory.  cd -f nn
	  is equivalent to cd ~{nn}.
- 13-06-06  In accordance with the standard set -u now causes failures for
	  unset positional parameters.
- 13-05-29  ksh93 now intercepts the `LC_TIME` variable.
- 13-05-29  namespace commands are no longer allowed inside function definitions
	  and now generate a syntax error.
- 13-05-10  With print -v for with nested compound variables, the output contains
	  typeset -C for sub-variables that are compound assignments.
- 13-05-08  Added a ksh -n options that suggests that x=$((expr)) be replaced by
	  ((x=expr))>
- 13-05-01 +Increased to maximum number of enumeration elements from 32K to
	  2G.
- 13-04-25 +The -K option has been added to set to sort indexed arrays of
	  compound elements based on a list of keys.
- 13-04-18  Added serialization to processing of CHLD traps.
- 13-04-15  ksh now waits for background jobs started in functions contained
	  in command substitution.
- 13-04-08 +ksh now sets .sh.sig.pid and .sh.sig.status for CHLD traps.  The
	  .sh.sig.status can be one of exited, killed, dumped, stopped or
	  continued.
- 13-04-08 +The CHLD trap is now triggered with STOP and CONT signals.
- 13-04-03 +Functions that are used in brace group command substitution ${ ... }
	  can assign the result to .sh.value instead of writing to standard
	  out with the same result, but faster.
- 13-03-27 +The variable .sh.sig containing siginfo information is no set during
	  a SIGCHLD trap.
- 13-03-12  Empty fields and empty arrays as type elements are not displayed
	  when expanding a type instance.
- 13-03-11  The trap command now blocks signals that have traps until the
	  trap command completes.
- 13-03-11  Signals that have traps that occur while processing a trap are
	  deferred until the trap command completes.
- 13-02-08 +The -p option was added to enum to display the values that are
	  valid for the enum.
- 13-02-07 +A preset alias named bool which is an alias for an enum
	  named `_Bool` which has values true and false has been added.
- 13-02-24  Increased the maximum level of recursion for evaluating variables
	  inside arithmetic expressions from 9 to 1024.
- 13-01-17 +User define math functions introduced on 10-03-24 now allow passing
	  arrays as arguments.
- 13-01-10 +ksh now treats ESC-O-letter the same as ESC-[-letter in vi and emacs
	  edit modes.
- 12-10-09 +read -d '' now reads up to a NUL byte.
- 12-10-04  libcmd builtins are statically linked into ksh93 and by default are
	  bound to the path /opt/ast/bin whether this path exists or not.
	  Changing the .sh.op_astbin variable changes the binding.
- 12-10-01 +Added the variable `SH_OPTIONS` which consists of name=value pairs.
	  For defined options it assigned value to the variable .sh.op_name.
- 12-10-02 +Add the variable .sh.op_astbin to define the directory where several
	  shell builtins will be bound.
- 12-09-10 +Added ~{fd} expansion where fd is the number of an open file or
	  a variable whose value is the number of an open file.
- 12-08-30  For an associative array A, $((A[sub])) no longer cause subscript sub
	  to be created.
- 12-08-20  typeset -H foo no longer un-sets foo when foo has been exported to
	  the shell.
- 12-07-12 +Added -q option was added to kill to send queued signals on systems
	  that support sigqueue().
- 12-07-12 +Added -p option to builtin to output builtins in a format that can
	  be used to re-input.
- 12-07-02 +The -a option was added to trap to cause the current setting to be
	  appended to the new trap setting rather than replacing it.
