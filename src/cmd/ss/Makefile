:PACKAGE: ast:static

LICENSE = since=1990,author=gsf

ss :: RELEASE ss.c -lcs

# On systems such as UnixWare, sys owns /dev/kmem.
$(LIBDIR) :INSTALLDIR: preserve=1 ssd
	ignore $(CP) $(*) $(<)
	if silent find /dev/kmem -group kmem >/dev/null 2>&1; then
		if $(CHGRP) kmem $(<); then
			ignore $(CHMOD) ug+s $(<)
		fi
	elif silent find /dev/kmem -group sys >/dev/null 2>&1; then
		if $(CHGRP) sys $(<); then
			ignore $(CHMOD) ug+s $(<)
		fi
	fi
	ignore $(CHMOD) +x $(<)

ssd :: ssd.8 ssd.c -lcs

$(INSTALLROOT)/share :INSTALLDIR:

$(INSTALLROOT)/share/lib/ss :INSTALLDIR:
