# AST

This is the AT&amp;T Software Technology (AST) toolkit from AT&amp;T Research.
It includes many tools and libraries, like KSH, NMAKE, SFIO, VMALLOC, VCODEX,
etc. It also includes more efficient replacements for a lot of the POSIX tools.
It was designed to be portable across many UNIX systems and also works
under UWIN on Microsoft Windows (see UWIN repo on GitHub under att/uwin).

## ksh93u+ and v-

This repo contains the **ksh93u+** and **ksh93v-** versions of KSH.

* **ksh93u+**, the master branch, was the last version released by the main AST
  authors in 2012, while they were at AT&amp;T. It also has some later build
  fixes but it is not actively maintained.
* ksh93v-, [ksh93v tag](https://github.com/att/ast/tree/ksh93v), contains
  contributions from the main authors through 2014 (after they left) and is
  considered less stable

Please search the web for forks of this repo (or check the
[Network graph](https://github.com/att/ast/network) on GitHub) if you are
looking for an actively maintained version of ksh.

## Build

This software is used to build itself, using NMAKE.  After cloning this repo, cd
to the top directory of it and run:

./bin/package make

Almost all the tools in this package (including the bin/package script are
self-documenting; run <tool> --man (or --html) for the man page for the tool.

(If you were used to the old AST packaging mechanism, on www.research.att.com,
this repo is equivalent to downloading the INIT and ast-open packages and
running: ./bin/package read on them).
