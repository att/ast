:PACKAGE: ast

LICENSE = zlib,since=1996,author="Julian R Seward"

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

bz 2.0 :LIBRARY: bzlib.h bzlib_private.h sfdcbzip.h \
	blocksort.c huffman.c crctable.c randtable.c compress.c \
	decompress.c bzlib.c sfdcbzip.c

"win32*" :NOOPTIMIZE: blocksort.c

$(INCLUDEDIR) :INSTALLPROTO: sfdcbzip.h bzlib.h

:: LICENSE README RELEASE
