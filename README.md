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
