:PACKAGE: ast

LICENSE = since=1996,author=kpv+gsf

VERSION = 1.0

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

recsort $(VERSION) :LIBRARY: RELEASE recsort.3 recsort.h rshdr.h \
		rsclear.c rsclose.c rsdisc.c rsnotify.c rslist.c rslib.c \
		rsmerge.c rsmethod.c rsopen.c rsprocess.c rswrite.c \
		rs-radix.c rs-rasp.c rs-splay.c rs-verify.c rs-copy.c \
		rskeyopen.c rskey.c rskeymeth.c rskeydump.c \
		rsfile.c rstemp.c \
		-ldll

$(INCLUDEDIR) :INSTALLPROTO: recsort.h

"win32*" :NOOPTIMIZE: rskey.c
