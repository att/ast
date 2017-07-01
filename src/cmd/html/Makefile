:PACKAGE: ast

LICENSE = since=1996,author=gsf

:ALL:

DEBUG == 1

html2rtf :: html2rtf.h html2rtf.c

html2db :: html2db.c

bb2tok :: bb2tok.c

htmlrefs :: htmlrefs.c

mm2html :: mm2html.sh

mm2bb :: mm2bb.sh

mm2foswiki :: mm2foswiki.sh

mm2twiki :: mm2twiki.sh

troff2html :: troff2html.h troff2html.c

"win32*" :NOOPTIMIZE: troff2html.c

$(LIBDIR)/html :INSTALLDIR: ast.mm strings.mm \
	man.tr mm.tr \
	mandoc.tr \
	mdoc.tr mdoc-common.tr mdoc-nroff.tr mdoc-syms.tr mdoc-troff.tr \
	download

download :: download.sh

:: RELEASE tst test.tr \
	old-troff2html.html old-troff2html.rtf \
	sh.1 old-sh.html old-sh.rtf \
	tour.mm old-tour.html old-tour.hpj old-tour.rtf
