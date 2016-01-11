/*
 * :PACKAGE: X11 support
 */

PACKAGE_X11_DIRS = \
	/usr/X11 $("/usr/X11([Rr][0-9]*([-.0-9]))":P=G:H=R) \
	/usr/openwin
PACKAGE_X11_VERSION = 6

if ! PACKAGE_X11 && ( ! PACKAGE_X11_INCLUDE || ! PACKAGE_X11_LIB )
	if ! ( PACKAGE_X11 = "$(PACKAGE_X11_DIRS:T=F:O=1)" )
		if ! PACKAGE_X11_INCLUDE
			PACKAGE_X11_INCLUDE := $(USRDIRS:/:/ /G:X=include/X11R$$(PACKAGE_X11_VERSION):T=F:O=1)
		end
		if ! PACKAGE_X11_LIB
			PACKAGE_X11_LIB := $(USRDIRS:/:/ /G:X=$$(CC.STDLIB.BASE:X=X11R$$(PACKAGE_X11_VERSION)):T=F:O=1)
		end
		if ! PACKAGE_X11_INCLUDE
			PACKAGE_X11_LIB =
		end
		if ! PACKAGE_X11_LIB
			PACKAGE_X11_INCLUDE =
		end
	elif ! PACKAGE_X11_LIB
		PACKAGE_X11_LIB := $(PACKAGE_X11:X=$$(CC.STDLIB.BASE:N!=lib))
	end
end

CC.REQUIRE.X11 = -lXext -lX11 -lnsl -lsocket -lw -ldl -lintl -ldnet_stub
CC.REQUIRE.Xaw = -lXaw -lXt
CC.REQUIRE.Xaw3d = -lXaw3d -lXt
CC.REQUIRE.Xm = -lXm -lXt
CC.REQUIRE.Xt = -lXt -lXmu -lX11 -lSM -lICE -lm

$(CC.REQUIRE.X11) \
$(CC.REQUIRE.Xaw) $(CC.REQUIRE.Xaw3d) \
$(CC.REQUIRE.Xm) $(CC.REQUIRE.Xt) \
: .DONTCARE
