:PACKAGE: ast

LICENSE = since=2002,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

netflow plugin=dss :LIBRARY: netflow.c \
	flow-dump.c flow-dumpv9.c flow-fixed.c flow-flat.c flow-tool.c \
	lib.c -ldss

$(INCLUDEDIR) :INSTALLPROTO: netflow.h

$(LIBDIR)/dss :INSTALLDIR: data/netflow-cflow.dss data/netflow-map.dss \
	data/netflow-sample.dss data/ip.map

$(LIBDIR)/pz :INSTALLDIR: dss-netflow.prt

:TEST: dss

:: RELEASE data/netflow-5.dat data/netflow-txt.dat
