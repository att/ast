#	Makefile for the Vczip command
#	Written by Kiem-Phong Vo

libs=	-lcdt -lsfio	# may be bypassed from above

INCDIR=	../../../../include
LIBDIR=	../../../../lib
LIBS=	-lvcodex $(libs)
INCS=	-I$(INCDIR) -I/usr/common/ast

CC=cc
CXINCS=
CCINCS= -I. -I.. -I$(INCDIR) $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)

vczip:	vczip.c $(LIBDIR)/libvcodex.a
	$(CC) $(CCFLAGS) vczip.c -L$(LIBDIR) $(LIBS) -o vczip

clean :
	-(rm vczip; exit 0) >/dev/null 2>&1
