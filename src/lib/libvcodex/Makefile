#########################################################################
#	Libvcodex.a makefile						#
#									#
#	Written by Kiem-Phong Vo					#
#########################################################################

# The below parameters should be edited as appropriate per installation.
# VCSFIO=1 if Sfio is used or 0 if Stdio is desired.
# VCPROPRIETARY=1 if various proprietary transforms are to be included.
# 	If VCPROPRIETARY=0, vc_prorprietary should be taken off "targets"
VCSFIO=0
VCPROPRIETARY=1
targets=vc_delta vc_huff vc_misc vc_proprietary

INCDIR=	../../../include
LIBDIR=	../../../lib
BINDIR=	../../../bin
MANDIR= ../../../man

CC=cc
CXINCS=
CCINCS=	-I. -I$(INCDIR) $(CXINCS)
CXFLAGS= 
CCMODE= -O
CCFLAGS= $(CCMODE) $(CXFLAGS) $(CCINCS)

#### Common Vcodex functions ############################################
VCSRCS=	vcopen.c vcclose.c vcio.c vcdisc.c vcextract.c vcrecode.c \
	vccontext.c vclog.c vcsfxsort.c vcperiod.c vclzparse.c \
	vcbuffer.c vcbcktsort.c vcqsort.c vcmethod.c vcsfio.c vcalias.c
VCOBJS=	vcopen.o vcclose.o vcio.o vcdisc.o vcextract.o vcrecode.o \
	vccontext.o vclog.o vcsfxsort.o vcperiod.o vclzparse.o \
	vcbuffer.o vcbcktsort.o vcqsort.o vcmethod.o vcsfio.o vcalias.o
VCHDRS=	vcodex.h

#### Delta compression ##################################################
VCDELTA=Vcdelta/vcdelta.o Vcdelta/vchamming.o \
	Vcdelta/vcdcache.o Vcdelta/vcdtable.o Vcdelta/vcdtblinit.o \
	Vcwindow/vcwopen.o Vcwindow/vcwclose.o Vcwindow/vcwfile.o \
	Vcwindow/vcwmethod.o Vcwindow/vcwngram.o Vcwindow/vcwmirror.o \
	Vcwindow/vcwvote.o Vcwindow/vcwdecode.o Vcwindow/vcwprefix.o

#### Huffman coding methods #############################################
VCHUFF=Vchuff/vchsize.o Vchuff/vchbits.o \
	Vchuff/vchtrie.o Vchuff/vchcode.o \
	Vchuff/vchuffman.o Vchuff/vchuffpart.o Vchuff/vchuffgroup.o

#### Miscellaneous methods ##############################################
VCMISC=Vcmisc/vcmap.o Vcmisc/vcrle.o Vcmisc/vcmtf.o Vcmisc/vcbwt.o \
	Vcmisc/vctranspose.o

#### Proprietary to AT&T only ###########################################
VCTABLE=Vctable/vctblplan.o Vctable/vctable.o Vctable/vcrtable.o
VCRDB=Vcrdb/vcrdplan.o Vcrdb/vcrdb.o Vcrdb/vcrdsepar.o \
	Vcrdb/vcrdparse.o Vcrdb/vcrdvector.o
VCSIEVE=Vcsieve/vcsieve.o
VCADHOC=Vcadhoc/vcama.o Vcadhoc/vcss7.o Vcadhoc/vcbdw.o \
	Vcadhoc/vcamadiff.o Vcadhoc/vcnetflow.o
VCPROPRI=$(VCTABLE) $(VCRDB) $(VCSIEVE) $(VCADHOC)

OBJS=	$(VCOBJS) $(VCDELTA) $(VCHUFF) $(VCMISC)
HDRS=	$(VCHDRS)

.c.o :
	$(CC) -c $(CCFLAGS) $*.c

all:	install

libvcodex.a: vcodex_h $(VCOBJS) $(targets)
	-(if test "$(VCPROPRIETARY)" = 1; then echo $(VCPROPRI) >kpv_propri; else echo >kpv_propri; fi)
	ar cr libvcodex.a $(OBJS) `cat kpv_propri; rm kpv_propri`;
	-(ranlib libvcodex.a; exit 0) >/dev/null 2>&1

vcodex_h: 
	-(Vcodex.ed $(VCSFIO) $(VCPROPRIETARY); exit 0) >/dev/null 2>&1

vc_delta: vc_window
	cd Vcdelta; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"
vc_window:
	cd Vcwindow; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vc_huff:
	cd Vchuff; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vc_misc:
	cd Vcmisc; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"

vc_proprietary: vc_table vc_rdb vc_sieve vc_adhoc
vc_table:
	cd Vctable; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"; 
vc_rdb:
	cd Vcrdb; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"; 
vc_sieve:
	cd Vcsieve; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"; 
vc_adhoc:
	cd Vcadhoc; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)";

install_me:	install
install:	install_lib install_cmd
install_lib:	libvcodex.a
	cp $(HDRS) $(INCDIR); \
	cp libvcodex.a $(LIBDIR); \
	(cp vcodex.3 $(MANDIR)/man3; exit 0) >/dev/null 2>&1; \
	(ranlib $(LIBDIR)/libvcodex.a; exit 0) >/dev/null 2>&1

install_cmd:
	cd Vczip; \
	$(MAKE) SHELL="$(SHELL)" CC="$(CC)" CCMODE="$(CCMODE)" \
		CXFLAGS="$(CXFLAGS)" CXINCS="$(CXINCS)"; 

clean :
	-(rm -rf $(OBJS) $(VCIOBJS) libvcodex.a Vczip/vczip; exit 0) >/dev/null 2>&1
