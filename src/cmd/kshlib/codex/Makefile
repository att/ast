:PACKAGE: --shared ast

LICENSE = since=2007,author=gsf

.SOURCE : ../../codex

main == b_codex

codex plugin=ksh :LIBRARY: codex.c lib.c -lcodex -lshell

:TEST: codex.tst $$(SHELL)

:: RELEASE
