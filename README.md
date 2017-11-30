KSH93
===

This repository contains ksh93 and libraries required to build it. It is a 
part of the AT&amp;T Software Technology (AST) toolkit from AT&amp;T Research.

Building
===
Building ksh93 requires meson build system.

To build it, execute below command from root directory of sources:

```
meson build
ninja -C build
```

Testing
===

Test cases can be run with:

```
meson test
```

If your system has an older version of Meson then :

```
mesontest
```

AST
===
Full AST source code is available under `2012-08-01-master` and `2016-01-10-beta`
branches. These branches are kept under low maintenance.

It includes many tools and libraries, like KSH, NMAKE, SFIO, VMALLOC, VCODEX,
etc. It also includes more efficient replacements for a lot of the POSIX tools.
It was designed to be portable across many UNIX systems and also works
under UWIN on Microsoft Windows (see UWIN repo on GitHub under att/uwin).

This software is used to build itself, using NMAKE.
After cloning this repo, cd to the top directory of it and run:

./bin/package make

Almost all the tools in this package (including the bin/package script are
self-documenting; run <tool> --man (or --html) for the man page for the tool.

(If you were used to the old AST packaging mechanism, on www.research.att.com,
this repo is equivalent to downloading the INIT and ast-open packages and
running: ./bin/package read on them).

Branches
========

The `master` [branch](https://github.com/att/ast/commits/master) contains current development version of ksh93. It only contains source code for ksh93 and the libraries required to build it.

The `2012-08-01-master` [branch](https://github.com/att/ast/commits/2012-08-01-master) contains the the last stable release of full AST source code. It contains one cherry-picked [change](https://github.com/att/ast/commit/e79c29295092fe2b2282d134e2b7cce32ec9dcac) to allow building on Linux.

The `2016-01-10-beta` [branch](https://github.com/att/ast/commits/2016-01-10-beta) contains the last beta release of full AST source code plus a number of patches that were later added on top of it.

The `gh-pages` [branch](https://github.com/att/ast/commits/gh-pages) will probably be removed. It contains files needed to generate the static [AST project page](https://att.github.io/ast/) hosted by Github. See https://help.github.com/articles/what-is-github-pages/ for more info.
