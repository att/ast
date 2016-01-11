:PACKAGE: ast

LICENSE = since=1998,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

pz 1.0 :LIBRARY: RELEASE pzip.3 pzip.h pzlib.h \
	pzopen.c pzhead.c pzpart.c \
	pzdeflate.c pzinflate.c pzlib.c pzinit.c \
	pzread.c pzwrite.c pzsync.c pzsplit.c \
	pzfixed.c pzconvert.c sfdcpzip.c sfdczip.c \
	-lbz -lz -ldll

$(INCLUDEDIR) :INSTALLPROTO: pzip.h

"win32*" :NOOPTIMIZE: pzread.c
