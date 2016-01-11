:PACKAGE: ast

LICENSE = since=1987,author=gsf

DEBUG == 1

pax :: RELEASE HISTORY PROMO.mm pax.1.posix \
	pax.h options.h \
	bio.c convert.c copy.c pax.c options.c \
	delta.c file.c format.c misc.c paxlib.c \
	pax-ar.c pax-bzip.c pax-compress.c \
	pax-cpio.c pax-delta.c pax-delta_88.c pax-flash.c \
	pax-gzip.c pax-ignore.c pax-mime.c pax-patch.c pax-pds.c \
	pax-rpm.c pax-saveset.c pax-slt.c pax-tar.c pax-tnef.c \
	pax-vczip.c pax-vdb.c pax-xz.c $(PAX.DEBUG.FORMAT) \
	-lardir -lodelta -lvdelta -lcodex -lsum -ldll

"win32*" :NOOPTIMIZE: delta.c

$(INCLUDEDIR) :INSTALLPROTO: paxlib.h

nocom :: nocom.c

$(LIBDIR)/pax :INSTALLDIR: cia.fmt ppar.fmt

:INSTALLDIR: bax testpax

if CC.HOSTTYPE != "win32*"
:INSTALLDIR: cpio tar
end

bax :: bax.1 bax.sh

cpio :: cpio.sh

delta2patch :: delta2patch.sh

rpm2cpio :: rpm2cpio.sh

tar :: tar.sh LICENSE=since=1989,author=gsf+brussell

:TEST: pax

:TEST: testpax.sh pax

::	data/link.dat data/mode.dat \
	data/aha.dat data/ro.dat \
	data/star-all-fail-1.dat \
	data/star-gnu-fail-1.dat \
	data/star-gnu-fail-2.dat \
	data/star-long-1.dat \
	data/star-long-1.lst \
	data/star-long-2.dat \
	data/star-long-2.lst \
	data/star-test.dat \
	data/y2k.dat \
	data/bigskip.pax \
	data/extract.pax \
	data/base.pax data/base-1.pax data/base-2.pax \
	data/delta.pax data/delta-1.pax data/delta-2.pax data/delta-3.pax \
	data/delta-4.pax data/delta-5.pax data/delta-6.pax data/delta-7.pax \
	data/file-01.pax data/file-02.pax data/file-03.pax
