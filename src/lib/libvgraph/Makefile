#	libgraph.a old make makefile
#	Written by Kiem-Phong Vo (02/01/2006)

INCDIR=	../../../include
LIBDIR=	../../../lib
BINDIR=	../../../bin

CXFLAGS=
CCMODE= -O
CCFLAGS= $(CCMODE) -I. -I$(INCDIR) $(CXFLAGS)
CC=cc

HDRS =	graph.h grhdr.h
SRCS =	graph.c grfold.c grbranching.c
OBJS =	graph.o grfold.o grbranching.o

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

all:	install

libgraph.a : $(OBJS)
	ar cr libgraph.a $(OBJS)
	-(ranlib libgraph.a; exit 0) >/dev/null 2>&1

install : libgraph.a
	-(rm -f $(INCDIR)/graph.h; exit 0) >/dev/null 2>&1
	-(rm -f $(LIBDIR)/libgraph.a; exit 0) >/dev/null 2>&1
	cp graph.h $(INCDIR);
	cp libgraph.a $(LIBDIR);
	-(ranlib $(LIBDIR)/libgraph.a; exit 0) >/dev/null 2>&1

3dupdate : $(SRCS) $(HDRS)
	cp Makefile $(SRCS) $(HDRS) ...

clean :
	-(rm $(OBJS) libgraph.a FEATURE/*; exit 0) >/dev/null 2>&1
