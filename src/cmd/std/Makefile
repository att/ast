:PACKAGE: ast

LICENSE = since=1989,author=gsf

:ALL:

:: PROMO.mm README RELEASE ls-color.sh ls-dir.sh

banner :: banner.c LICENSE=since=1989,author=dgk

dd :: dd.c

df :: df.c $(CC.HOSTTYPE:N=sun4:?+last??)

du :: du.c

file :: file.c

locale :: locale.c

mesg :: mesg.c LICENSE=since=1995,author=dgk

$(PACKAGE_LOCAL)mount :: mount.c LICENSE=since=1998,author=gsf $(CC.HOSTTYPE:N=sun4:?+last??)

$(PACKAGE_LOCAL)umount :LINK: $(PACKAGE_LOCAL)mount

ps :: ps.c pss.c pss-null.c LICENSE=since=1998,author=gsf \
	pss-procfs.c pss-getprocs.c pss-cygwin.c pss-kvm.c pss-pstat.c \
	pss-ps.c pss-info.c $(CC.HOSTTYPE:N=(sol|unixware).*:??-lkvm?)

dontcare -lkvm

seq :: seq.sh LICENSE=since=2007,author=dgk

shar :: shar.sh LICENSE=since=1999,author=dgk

split :: split.c LICENSE=since=1995,author=dgk+gsf

csplit :LINK: split

touch :: touch.c

tsort :: tsort.c LICENSE=since=1995,author=dgk+gsf

unexpand :: unexpand.c LICENSE=since=1995,author=dgk

expand :LINK: unexpand

yes :: yes.sh LICENSE=since=2007,author=dgk

:TEST: csplit dd locale seq touch tsort

:: y2k.dat
