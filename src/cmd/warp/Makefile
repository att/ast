:PACKAGE: ast:static

LICENSE = since=1998,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

VERSION = 1.0

DEBUG == 1

warp $(VERSION) :LIBRARY: warp.c +ldll -ldl

warp :: warp.sh

:: README RELEASE PROMO.mm

:TEST: warp
