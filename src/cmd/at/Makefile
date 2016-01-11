:PACKAGE: ast

LICENSE = since=1996,author=gsf

DEBUG == 1

if CC.HOSTTYPE == "win32*"
AT_STRICT == 1
PACKAGE_LOCAL ==
end

$(PACKAGE_LOCAL)at :: PROMO.mm RELEASE at.h at.c -lcoshell -lcs

$(PACKAGE_LOCAL)batch :LINK: $(PACKAGE_LOCAL)at

$(LIBDIR)/at/jobs :INSTALLDIR: mode=u=rwx,go=x

$(LIBDIR)/at/jobs :INSTALLDIR: mode=u=rx,go=x atx

atx :: atx.c +last

at :SERVICE: atd.c -lcs +last

$(PACKAGE_LOCAL)crontab :: crontab.sh
