:PACKAGE: ast

LICENSE = since=2003,author=gsf

CCFLAGS = $(CC.DLL) $(CC.OPTIMIZE)

sync plugin=sort :LIBRARY: sync.c ss.c -lrecsort

test plugin=sort :LIBRARY: test.c

$(INCLUDEDIR) :INSTALLDIR: ssexit.h

:TEST: sort

:: RELEASE NOTES \
	data/c.dat data/c.ss \
	data/e.dat data/e%12.dat data/e.f%12.dat data/e.ss \
	data/f.ss \
	data/g.ss \
	data/h.ss \
	data/i.dat data/i.ss \
	data/k.ss \
	data/p.dat data/p.ss \
	data/r.ss \
	data/s.dat data/s.ss \
	data/t.dat data/t.ss \
	data/u.ss \
	data/v.ss \
	data/x.ss \
	data/z.dat data/z.ss
