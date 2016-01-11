:PACKAGE: ast

LICENSE = since=2003,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

rar 3.0 plugin=pax :LIBRARY: rar.c -lcodex

"win32*" :NOOPTIMIZE: rar.c

:: RELEASE README
