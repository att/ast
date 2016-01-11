/*
 * connect stream library
 */

:PACKAGE: ast

LICENSE = since=1990,author=gsf

ID = cs

DEBUG == 1

CCFLAGS = $(CC.OPTIMIZE) $(CC.SUFFIX.DYNAMIC:@?$(CC.DLL)??)

TRUST == "$(INSTALLROOT):$(OPTDIRS:/:/ /G:C%$%/$(.PACKAGE.:O=1)%:/ /:/G):$(USRDIRS)"

HEADERS = cs.h css.h msg.h ast_cs.h cs_lib.h

$(ID) 3.0 :LIBRARY: RELEASE cs.3 \
	$(HEADERS) cslib.h csslib.h \
	csalloc.c csfree.c \
	csaddr.c csattr.c csauth.c csbind.c csclient.c cscontrol.c \
	csdaemon.c csdata.c csfrom.c csfull.c csinfo.c cslocal.c \
	csname.c csnote.c csntoa.c csopen.c cspath.c cspeek.c \
	csping.c cspipe.c cspoll.c csport.c csread.c csrecv.c \
	cssend.c css.c csserve.c csstat.c csto.c csvar.c cswrite.c \
	msglib.h msgblast.c msgbuf.c msggetmask.c msgindex.c \
	msginfo.c msglist.c msgname.c msgread.c msgrecv.c msgsend.c \
	msgsetmask.c msguser.c \
	-lin -lipc -lnetcompat \
	$(CC.HOSTTYPE:N=$(-mam:N=static*:?UNKNOWN?sgi.mips*?):??-lsocket -linet -lnsl -ldl?)

$(HEADERS:N=*_*.h) :COPY: FEATURE/$$(<:B:/.*_//)

$(INCLUDEDIR) :INSTALLPROTO: $(HEADERS)

$(LIBDIR)/$(ID) :INSTALLDIR: genlocal genshare

$(SHAREDIR)/lib/$(ID) :INSTALLDIR: local.ast genlocal genshare

$(SHAREDIR)/lib/$(ID)/tcp :INSTALLDIR: mode=+rwxt

$(SHAREDIR)/lib/$(ID)/udp :INSTALLDIR: mode=+rwxt

$(LIBDIR)/make :INSTALLDIR: SERVICE.mk

/*
 * share and local will most likely require human intervention
 * so we generate minimal files with obnoxious warnings
 */

genlocal :: genlocal.sh

genshare :: genshare.sh

share : genshare FEATURE/lib $(SHAREDIR)
	if	test ! -f $(<)
	then	{
		rsh=`$(SED) -e '/^#define CS_REMOTE_SHELL/!d' -e 's/.*"\(.*\)".*/\1/' $(*:O=2)`
		echo "#!!! 1 to update this file: $INSTALLROOT/lib/$(ID)/$(*:O=1) -d $(*:O=3) -r $rsh > $(<) !!!"
		} > $(<)
	fi

"local" : genlocal FEATURE/lib share
	if	test ! -f $(<)
	then	{
		rsh=`$(SED) -e '/^#define CS_REMOTE_SHELL/!d' -e 's/.*"\(.*\)".*/\1/' $(*:O=2)`
		echo "#!!! 1 to update this file: $INSTALLROOT/lib/$(ID)/$(*:O=1) -r $rsh > $(<) !!!"
		} > $(<)
	fi

$(LIBDIR)/$(ID) :INSTALLDIR: share
	if	test ! -f $(<)
	then	$(CP) $(*) $(<)
	fi

$(SHAREDIR)/lib/$(ID) :INSTALLDIR: local
	if	test ! -f $(<)
	then	$(CP) $(*) $(<)
	fi
