:PACKAGE: ast

LICENSE = since=1993,author=dgk

pack :: pack.c -lhuff

unpack :: unpack.c -lhuff

pcat :: unpack.c -lhuff

huff 1.0 :LIBRARY: huff.3 huffman.h \
		huffinit.c huffgethdr.c huffputhdr.c \
		huffencode.c huffdecode.c

:TEST: pack

:: RELEASE pack.dat
