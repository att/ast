#	Makefile for the Huffman methods
#	Written by Kiem-Phong Vo

INCDIR=	../../../../include
LIBDIR=	../../../../lib

CXINCS=
CCINCS= -I$(INCDIR) -I. -I.. $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)
CC=cc

VCHSRCS=	vchsize.c vchbits.c vchtrie.c vchcode.c \
		vchuffman.c vchuffpart.c vchuffgroup.c
VCHOBJS=	vchsize.o vchbits.o vchtrie.o vchcode.o \
		vchuffman.o vchuffpart.o vchuffgroup.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

install:	$(VCHOBJS)
clean :
	-(rm $(VCHOBJS); exit 0) >/dev/null 2>&1
