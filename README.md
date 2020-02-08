# This branch is not supported

This branch contains the **ksh2020** version of the AST tools, from January
2020, which primarily written and maintained by @krader1961 and @siteshwar.

This branch is not supported or maintained by AT&amp;T; the repo at
https://github.com/att/ast only officially hosts ksh93u+ and the experimental
ksh93v- branch.

See [#1464](https://github.com/att/ast/issues/1464) and
[#1466](https://github.com/att/ast/issues/1466) for discussion and history of
this decision.

Please search for forks of this repo if you are looking for ksh2020.


# KSH93

This repository contains the AT&amp;T Software Technology (AST) toolkit
from AT&amp;T Research.  As of November 2017 the development focus has
been shifted to the `ksh` (or `ksh93`) command and supporting code required
to build it.

The non-ksh code of the AST project is no longer being actively
maintained. If you are interested in the non-ksh code see below for
details on which branches contain the full AST code base.

The project only supports systems where the compiler and underlying
hardware is ASCII compatible. This includes Linux on IBM zSeries but not
z/OS. The nascent, incomplete, support for EBCDIC has been removed. See
[issue #742](https://github.com/att/ast/issues/742).

## Building Korn shell

Building ksh requires the [Meson](http://mesonbuild.com/) build system. To
build ksh execute these commands from the project root directory:

```
meson build
ninja -C build
```

You can add a `--prefix` flag followed by a path to the `meson build` command
to specify where the binaries and libraries are installed. The default is
*/usr/local*.

## Installing

The `ksh` executable, helper libraries and man page can be installed with:

```
ninja -C build install
```

## Getting KornShell

### Linux
Latest builds for Red Hat Enterprise Linux (CentOS) and Fedora can be found in
[Copr](https://copr.fedorainfracloud.org/coprs/g/ksh/latest/).

Latest builds for openSUSE and Ubuntu can be found in
[OBS](https://build.opensuse.org/project/show/shells:ksh:new:latest).

See [full list](https://github.com/att/ast/wiki/Packages-for-Linux) of distros
that ship packages for current development version.

### Windows
TBD

### macOS
Using [MacPorts](https://www.macports.org), the latest stable version can be installed using:

```
sudo port install ksh
```

Alternately, a development version can be installed using:

```
sudo port install ksh-devel
```

## Versioning

We have switched from the legacy versioning scheme to
[semantic versioning](https://github.com/att/ast/issues/335). The first
release after the last stable, ksh93u+, release is version 2020.0.0 which has
numeric version number 20200000. This means you can still evaluate
`$KSH_VERSION` or `${.sh.version}` in numeric context to determine if the
current shell is older or newer than a specific release (e.g., 20120801 for
the ksh93u+ release).

If the shell is built without Git metadata a hardcoded value is used. That
value will be one of two things. If building from a stable release branch
it will be whatever value was assigned to that branch; e.g., 2020.0.1.
If building from the master, experimental, branch it will be the most
recent major release number and 99 for the minor and patch levels; e.g.,
2020.99.99. This should serve as a warning that you don't really know
what commit was used when building ksh.

## Contributing changes to the source code

See the
[contributing guidelines](https://github.com/att/ast/blob/master/CONTRIBUTING.md)
for information about code style, unit tests, and other topics of interest
to anyone who wants to modify the source.

## Guidelines for Packagers

See the guidelines for downstream package maintainers [here](https://github.com/att/ast/wiki/Guidelines-for-Packagers).

## Coverity Scan

Latest results of coverity scan can be viewed [here](https://scan.coverity.com/projects/ksh).

## Test Coverage

A code coverage report can be viewed
[here](http://situ.im/ast/coveragereport/) for GCC on Linux, and
[here](https://www.skepticism.us/ast/coveragereport/) for LLVM/clang on macOS.

## Working with the full AST source

Warning: Running test cases through the legacy test script may wipe out your home directory
([See #477](https://github.com/att/ast/issues/477)). This has been fixed in current
development version.

Full AST source code is available under the `2012-08-01-master` and
`2016-01-10-beta` branches (see below for more details). Bug fixes to
these branches are welcome but they are otherwise under low maintenance.

The full AST code includes many tools and libraries, like KSH, NMAKE, SFIO,
VMALLOC, VCODEX, etc. It also includes efficient replacements for a
lot of the POSIX tools.  It was designed to be portable across many UNIX
systems and also works under UWIN on Microsoft Windows (see UWIN repo on
GitHub under att/uwin).

This software is used to build itself, using NMAKE.
After cloning this repo, cd to the top directory of it and run:

```
./bin/package make
```

__NOTE__: That build command only applies to the legacy branches that include
the entire AST source. It does not apply to the current `master` branch that
is focused solely on the `ksh` command and uses the Meson build system.


Almost all the tools in this package (including the bin/package script) are
self-documenting; run <tool> --man (or --html) for the man page for the tool.

(If you were used to the old AST packaging mechanism, on www.research.att.com,
this repo is equivalent to downloading the INIT and ast-open packages and
running: ./bin/package read on them).

## Branches

The `master` [branch](https://github.com/att/ast/commits/master) contains
the current development version of ksh93. It only contains source code
for ksh93 and the libraries required to build it.

The [2012-08-01-master branch](https://github.com/att/ast/commits/2012-08-01-master)
contains the last stable release, ksh93u+,
of full AST source code. It contains one cherry-picked
[change](https://github.com/att/ast/commit/e79c29295092fe2b2282d134e2b7cce32ec9dcac)
to allow building on Linux.

The [2016-01-10-beta branch](https://github.com/att/ast/commits/2016-01-10-beta) contains
the last beta release, ksh93v-, of full AST source code plus a number of patches
that were later added on top of it. Changes for the legacy AST project are
normally merged to this branch.

The [gh-pages branch](https://github.com/att/ast/commits/gh-pages)
will probably be removed. It contains files needed to generate the static
[AST project page](https://att.github.io/ast/) hosted by GitHub. See
https://help.github.com/articles/what-is-github-pages/ for more info.

## Licensing

This section needs more detail and clarity. The original `README` document
said the following:

> Each package is covered by one of the license files
>
>   lib/package/LICENSES/<license>
>
> where <license> is the license type for the package.  At the top
> of each license file is a URL; the license covers all software that
> refers to this URL. For details run
>
>   bin/package license [<package>]
>
> Any archives, distributions or packages made from source or
> binaries covered by license(s) must contain the corresponding
> license file(s).

## Contact Us

The primary mechanism for interacting with this project is the [GitHub
project](https://github.com/att/ast/) by opening issues and pull-requests
and watching the project.

### IRC

Use `#ksh` channel on freenode to discuss about AT&T KornShell and related variants.

### Gitter

[ksh93/users](https://gitter.im/ksh93/users) is the Gitter chatroom for
AT&T KornShell users.

### Mailing lists

You can track announcements and ask question or
report problems at korn-shell@googlegroups.com or
https://groups.google.com/d/forum/korn-shell as an alternative to watching
this Github project.

Archives of the AST mailing lists (which is mostly about ksh) are available at:
https://marc.info/?l=ast-developers and https://marc.info/?l=ast-users.

__As of 2018-11-01 the AT&T hosted mailing lists no longer respond to
email.__ Note that the precise date the mailing lists stopped working
is unknown. That was simply the date it came to our attention. The last
known message to both mailing lists was sent on 2018-03-15. Ironically,
since you can't unsubscribe, they still send monthly reminders that you
are subscribed.
