/*
 * coshell library
 */

:PACKAGE: ast

CCFLAGS = $(CC.OPTIMIZE) $(CC.PIC) /* pic allows archive to be pulled into other dlls */

LICENSE = since=1990,author=gsf

coshell 1.0 :LIBRARY: RELEASE coshell.3 coshell.h colib.h \
		coopen.c coclose.c coinit.c coexec.c costash.c \
		cowait.c cokill.c cosync.c coquote.c codata.c \
		coexport.c procrun.c system.c

ignore :: ignore.sh

silent :: silent.sh

$(INCLUDEDIR) :INSTALLPROTO: coshell.h
