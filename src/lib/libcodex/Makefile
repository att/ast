:PACKAGE: ast

LICENSE = since=2003,author=gsf

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

codex 1.0 :LIBRARY: codex.c codexdata.c codexid.c codexlist.c \
	codexcmp.c codexmeth.c codexadd.c codexnull.c \
	codexgetpass.c codexpass.c codexsize.c codexoptinfo.c \
	code-rot13.c code-qp.c code-uu.c code-iconv.c code-vcodex.c \
	-lvcodex -ldll

$(INCLUDEDIR) :INSTALLPROTO: codex.h

:: RELEASE NOTES
