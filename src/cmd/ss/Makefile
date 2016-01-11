:PACKAGE: ast:static

LICENSE = since=1990,author=gsf

ss :: RELEASE ss.c -lcs

$(LIBDIR) :INSTALLDIR: group=kmem mode=ug+s,+x preserve=1 ssd

ssd :: ssd.8 ssd.c -lcs

$(INSTALLROOT)/share :INSTALLDIR:

$(INSTALLROOT)/share/lib/ss :INSTALLDIR:
