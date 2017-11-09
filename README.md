AST
===

This is the AT&amp;T Software Technology (AST) toolkit from AT&amp;T Research.
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

The `2012-08-01-master` [branch](https://github.com/att/ast/commits/2012-08-01-master) contains the the last stable release. It contains one cherry-picked [change](https://github.com/att/ast/commit/e79c29295092fe2b2282d134e2b7cce32ec9dcac) to allow building on Linux.

The `2016-01-10-beta` [branch](https://github.com/att/ast/commits/2016-01-10-beta) contains the last beta release plus a number of patches that were later added on top of it. It was forked from the last commit made to the `beta` branch before the focus of that branch was switched to the `ksh` command and code not relevant to its maintanence started being removed.

The `beta` [branch](https://github.com/att/ast/commits/master) is focused on maintaining the ksh command. Every attempt is made to avoid introducing regressions or backward incompatible changes but it should be considered unstable. Code not needed to build and test ksh will be removed as time permits to make it easier to focus on ksh. If you need other AST libraries or commands you should use the branches listed above. At some point in the near future `beta` will be folded into `master`.

The `master` [branch](https://github.com/att/ast/commits/master) is currently unused. In the near future the `beta` branch will be folded into it and bleeding edge development of the `ksh` command will be done in this branch.

The `gh-pages` [branch](https://github.com/att/ast/commits/gh-pages) will probably be removed. It contains files needed to generate the static [AST project page](https://att.github.io/ast/) hosted by Github. See https://help.github.com/articles/what-is-github-pages/ for more info.
