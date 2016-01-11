:PACKAGE: ast ip_t:order

LICENSE = since=2002,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

bgp plugin=dss static :LIBRARY: \
		bgp.c lib.c \
		bgp-fixed.c bgp-cisco.c bgp-ciscov6.c \
		bgp-mrt.c bgp-mrt-anonymize.c \
		bgp-ipma.c bgp-table.c \
		-ldss

$(LIBDIR)/dss :INSTALLDIR: data/bgp-map.dss data/as.map data/bgp.map

$(LIBDIR)/pz :INSTALLDIR: dss-bgp.prt

:TEST: dss

:: RELEASE MRT RFC \
	data/as.map           data/ipma-lcl.dat  data/rib.20080601.0108.bz2 \
	data/bgp-map.dss      data/ipma-tab.dat  data/rib.20090316.2200.dat \
	data/bgp.map          data/ipma-txt.dat  data/ripe-08.dat \
	data/cisco-split.dat  data/ipma.dat      data/updates.20080601.0000.bz2 \
	data/cisco.dat        data/mrt.dat \
	data/a01.mrt          data/a01.out \
	data/a02.mrt          data/a02.out \
	data/a03.mrt          data/a03.out \
	data/a04.mrt          data/a04.out \
	data/a05.mrt          data/a05.out \
	data/a06.mrt          data/a06.out \
	data/a07.mrt          data/a07.out \
	data/a08.mrt          data/a08.out \
	data/a09.mrt          data/a09.out \
	data/a10.mrt          data/a10.out \
	data/a11.mrt          data/a11.out \
	data/a12.mrt          data/a12.out \
	data/a13.mrt          data/a13.out \
	data/a14.mrt          data/a14.out \
	data/a15.mrt          data/a15.out \
	data/f01.mrt          data/f01.out \
	data/mvpn-key-01.dat
