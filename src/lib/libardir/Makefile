:PACKAGE: ast

LICENSE = since=2002,author=gsf+dgk

ardir :LIBRARY: ardir.c \
	ar-aix.c ar-aixbig.c ar-local.c ar-omf.c \
	ar-pdp11.c ar-port.c ar-s5r0.c

$(INCLUDEDIR) :INSTALLPROTO: ardir.h

:INSTALLDIR: testar

testar :: testar.c -lardir

:: RELEASE
