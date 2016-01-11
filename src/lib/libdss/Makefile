:PACKAGE: ast

LICENSE = since=2002,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

_BLD_cx == 1
_BLD_tag == 1

dss 1.0 :LIBRARY: dssopen.c dssfile.c dssprintf.c dssmisc.c\
		dssmagic.c dssopt.c dsstags.c \
		tag.c \
		cxopen.c cxcomp.c cxeval.c cxmap.c cxattr.c \
		-lpz -ldll

if CC.HOSTTYPE == "sol*.sun4" || "$(-mam)" == "static*"
dsslib.o cxeval.o cxopen.o cxparse.o : CCFLAGS="$(CC.DEBUG) $(CC.DLL)"
end

$(INCLUDEDIR) :INSTALLPROTO: cx.h cxlib.h dss.h dsslib.h tag.h

:: RELEASE NOTES
