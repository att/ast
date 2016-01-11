:PACKAGE: ast ksh93:force

LICENSE = since=2007,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

dbm plugin=ksh :LIBRARY: dbm.c -ldbm -lshell

:TEST: dbm.tst $$(SHELL)

:: RELEASE
