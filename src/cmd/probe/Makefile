:PACKAGE: ast

LICENSE = since=1989,author=gsf

$(LIBDIR)/probe :INSTALLDIR: mode=u+s,+x probe

$(BINDIR) :INSTALLDIR: mode=u+s,+x probe

probe :: RELEASE probe.c +last

"ibm.risc"      :NOOPTIMIZE: probe.c
