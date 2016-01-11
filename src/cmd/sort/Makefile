:PACKAGE: ast

LICENSE = since=1996,author=gsf+kpv+doug

sort :: README RELEASE PROMO.mm main.c \
	-lrecsort -ldll -lz

:INSTALLDIR: gen rec testsort

gen :: LICENSE=since=1998,author=gsf gen.c

rec :: LICENSE=since=2005,author=gsf rec.c

:TEST: sort

script :TEST: testsort.sh sort

:: sfopen.c
