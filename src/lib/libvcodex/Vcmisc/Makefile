#	Makefile for the miscellaneous methods
#	Written by Kiem-Phong Vo

INCDIR=	../../../../include
LIBDIR=	../../../../lib

CXINCS=
CCINCS= -I$(INCDIR) -I. -I.. $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)
CC=cc

VCMSRCS=	vcmap.c vcrle.c vcmtf.c vcbwt.c vctranspose.c
VCMOBJS=	vcmap.o vcrle.o vcmtf.o vcbwt.o vctranspose.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

install:	$(VCMOBJS)
clean :
	-(rm $(VCMOBJS); exit 0) >/dev/null 2>&1
