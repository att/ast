/*
 * NOTE: this provides ast enhanced versions of INIT executables
 */

:PACKAGE: ast

LICENSE = since=1990,author=gsf

.SOURCE.c : ../INIT ../../lib/libpp

proto :: RELEASE PROMO.mm proto.c

mamake :: mamake.c LICENSE=since=1999,author=gsf

ratz :: ratz.c

release :: release.c LICENSE=since=1999,author=gsf

sear :: sear.sh LICENSE=since=2001,author=dgk+gsf

changes :: changes.sh LICENSE=since=2005,author=gsf

$(LIBDIR)/sear :INSTALLDIR: ratz.c sear.ico sear32.ico sear64.ico

:TEST: proto

/*
 * ../INIT/proto.c is snarfed from the standalone src
 * this is a gross hack to allow only one copy of the source
 */

if ! "$(-mam)"

../INIT/proto.c :INSTALL: .SCAN.NULL proto.c
	if	grep '^#pragma prototyped' $(*) >/dev/null 2>/dev/null
	then	{
		echo "#define __PROTO__(x) x"
		echo "#define __MANGLE__"
		$(SED) \
		-e '/\/\*DELAY_CONTROL\*\//,/\/\*NODELAY_CONTROL\*\//s/^#/DELAY_CONTROL /' \
		-e '/\/\*DELAY_CONTROL\*\//s/^/#define DELAY_CONTROL # \
	#undef	__STDC__/' \
		-e '/\/\*NODELAY_CONTROL\*\//s/^/#undef	DELAY_CONTROL /' \
		$(*)
		} > a.$(tmp).c
		$(CPP) $(CPPFLAGS) -I$(INSTALLROOT)/include/ast -DPROTO_STANDALONE -D-F$(*) -D-L -I-D -E a.$(tmp).c |
		$(SED) -e 's%".*/src/cmd/proto/%"%' -e 's%".*/src/cmd/%"../%' -e 's%".*/src/lib/%"../../lib/%' |
		$(PROTO) $(PROTOFLAGS:N!=-s) -f > b.$(tmp).c
		if	test '' != '$(<:P=L*:O=N)' && $(CMP) -s b.$(tmp).c $(<:P=L*:O=N)
		then	: no changes for $(<) :
		elif	$(CC) -o b.$(tmp).x b.$(tmp).c
		then	$(CP) b.$(tmp).c $(<)
		else	: retain original $(<) :
		fi
		$(RM) -f ?.$(tmp).?
	fi

end
