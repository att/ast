if ! ( CC.HOSTTYPE == "*win*" && CC.SUFFIX.DYNAMIC == ".dll" || "$(PACKAGE_OPTIONS:N=optimize-space)" )
	libtype = :static
end

:PACKAGE: - X11 tk tcl shell$(libtype):20120101 ast$(libtype)

CCFLAGS = $(CC.OPTIMIZE)

LICENSE = since=1996,author=dgk+jlk

tksh :: tksh.1 uinit.c tkMain.c

:: PROMO.mm README RELEASE
