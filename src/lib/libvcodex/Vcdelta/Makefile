#	Makefile for the Vcdelta method
#	Written by Kiem-Phong Vo

INCDIR=	../../../../include
LIBDIR=	../../../../lib

CXINCS=
CCINCS= -I$(INCDIR) -I.. -I. $(CXINCS)
CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)
CC=cc

VCDHDRS= vcdhdr.h
VCDSRCS= vcdelta.c vcdcache.c vcdtblinit.c vcdtable.c vchamming.c
VCDOBJS= vcdelta.o vcdcache.o vcdtblinit.o vcdtable.o vchamming.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

install_me:	$(VCDOBJS)
install:	install_me

# 3d update
update:
	cp Makefile $(VCDHDRS) $(VCDSRCS) ...

clean :
	-(rm $(VCDOBJS); exit 0) >/dev/null 2>&1
