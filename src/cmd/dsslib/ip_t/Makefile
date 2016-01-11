:PACKAGE: ast

LICENSE = since=2000,author=gsf+kpv

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

_BLD_pt == 1

ip_t plugin=dss static :LIBRARY: iv.3 ptv.3 \
	ip_t.c lib.c \
	ire.c itl.c \
	ptaddresses.c ptcopy.c ptcover.c ptdifference.c ptequal.c \
	ptintersect.c ptinvert.c ptmatch.c ptopen.c ptprint.c ptranges.c \
	ptsize.c ptstats.c ptsubset.c ptunion.c ptdump.c ptrebit.c \
	fv.c \
	ivopen.c ivclose.c ivstr.c ivfmt.c ivmeth.c iv-flat.c iv-nested.c \
	ptvaddresses.c ptvcopy.c ptvcover.c ptvdifference.c ptvdump.c \
	ptvequal.c ptvintersect.c ptvinvert.c ptvmatch.c ptvmin.c ptvmax.c \
	ptvopen.c ptvprint.c ptvranges.c ptvrebit.c ptvsize.c ptvstats.c \
	ptvsubset.c ptvunion.c \
	-ldss

$(INCLUDEDIR) :INSTALLPROTO: bgp.h fv.h iv.h ivmethods.h pt.h ptv.h

iv :TEST: testiv \
		iv-ip4-prefix.dat iv-ip4-addr.dat iv-ip4.out \
		iv-ip6-prefix.dat iv-ip6-addr.dat iv-ip6.out
	set +x $(*:O>1)
	while	:
	do	case $# in
		[012])	break ;;
		esac
		case $1 in
		*-ip4-*)f=-ip4 ;;
		*)	f=-ip6 ;;
		esac
		$(*:O=1) $f $1 $2 > iv$f.tmp
		if	cmp -s iv$f.tmp $3
		then	r=passed
		else	r=failed
		fi
		echo iv $f test $r
		shift 3
	done

:INSTALLDIR: bgpsize testiv testptv

bgpsize :: bgpsize.c

testiv :: testiv.c -ldssip_t

testptv :: testptv.c -ldssip_t

PAXFILTER = ;*.[ch];$(PROTO) $(PROTOFLAGS) -c "" -d -p

ivsa.tgz ptvsa.tgz : $$(<:B:S=.manifest:T=F:T=I:/[[:space:]][[:space:]]*/ /G)
	$(PAX) $(PAXFLAGS) -wf $(<) -x $(<:/.*\.//) -s ',.*/,,' -A $(PAXFILTER:@Q) $(*)

:: RELEASE \
	README-ivsa ivsa.manifest ivsa.mm ivsa.omk \
	README-ptvsa ptvsa.manifest ptvsa.mm ptvsa.omk
