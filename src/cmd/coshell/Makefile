:PACKAGE: ast

LICENSE = since=1990,author=gsf

DEBUG == 1
PATHCHECK == CO_ID

coshell fdp $(BINDIR) :SERVICE: RELEASE PROMO.mm coshell.1 local.ast \
		service.h main.c job.c schedule.c shell.c command.c \
		misc.c init.c -lcoshell

event tcp $(BINDIR) :SERVICE: event.c -lcs -ldbm

$(FUNDIR) :INSTALLDIR: mode=+x cosh title

:INSTALLDIR: cotest

cotest :: cotest.c schedule.o misc.o -lcs

$(LIBDIR)/make :INSTALLDIR: COSHELL.mk
