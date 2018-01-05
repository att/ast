# KSH93

This repository contains the AT&amp;T Software Technology (AST) toolkit
from AT&amp;T Research.  As of November 2017 the development focus has
been shifted to the `ksh` (or `ksh93`) command and supporting code required
to build it.

The non-ksh code of the AST project is no longer being actively
maintained. If you are interested in the non-ksh code see below for
details on which branches contain the full AST code base.

## Building Korn shell

Building ksh requires the [Meson](http://mesonbuild.com/) build system. To
ksh execute these commands from the project root directory:

```
meson build
ninja -C build
```

## Working with the full AST source

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

Almost all the tools in this package (including the bin/package script are
self-documenting; run <tool> --man (or --html) for the man page for the tool.

(If you were used to the old AST packaging mechanism, on www.research.att.com,
this repo is equivalent to downloading the INIT and ast-open packages and
running: ./bin/package read on them).

## Branches

The `master` [branch](https://github.com/att/ast/commits/master) contains
the current development version of ksh93. It only contains source code
for ksh93 and the libraries required to build it.

The [2012-08-01-master branch](https://github.com/att/ast/commits/2012-08-01-master)
contains the the last stable release of
full AST source code. It contains one cherry-picked
[change](https://github.com/att/ast/commit/e79c29295092fe2b2282d134e2b7cce32ec9dcac)
to allow building on Linux.

The [2016-01-10-beta branch](https://github.com/att/ast/commits/2016-01-10-beta) contains
the last beta release of full AST source code plus a number of patches
that were later added on top of it. Changes for the legacy AST project are
normally merged to this branch.

The [gh-pages branch](https://github.com/att/ast/commits/gh-pages)
will probably be removed. It contains files needed to generate the static
[AST project page](https://att.github.io/ast/) hosted by Github. See
https://help.github.com/articles/what-is-github-pages/ for more info.

## Contact Us

The primary mechanism for interacting with this project is the [Github
project](https://github.com/att/ast/) by opening issues and pull-requests
and watching the project.

TBD is setting up a public Gitter, IRC, and other channels for real-time
communication.

Archives of the legacy AST mailing lists (which is mostly
about ksh) can be read at https://marc.info/?l=ast-developers and
https://marc.info/?l=ast-users. These mailing lists now receive almost
zero traffic. It no longer appears to be possible to subscribe to those
mailing lists.
