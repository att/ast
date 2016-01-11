:PACKAGE: ast

CCFLAGS = $(CC.OPTIMIZE) $(CC.PIC) /* pic allows archive to be pulled into other dlls */

DEBUG == 1

LICENSE = since=2003,author=gsf

jcl :LIBRARY: open.c parse.c run.c sym.c path.c find.c lib.c uniq.c \
	-lcoshell

$(INCLUDEDIR) :INSTALLPROTO: jcl.h

$(LIBDIR)/jcl :INSTALLDIR:

:: RELEASE NOTES
