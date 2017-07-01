/*
 * standalone versions of -lcmd builtins
 */

:PACKAGE: ast cmd:space

LICENSE = since=1992,author=gsf+dgk

:ALL:

:: cmdlib.h builtin.c

CMD_BUILTIN ==
CMD_DYNAMIC ==
CMD_STANDALONE ==

LINKS = cp: ln mv cksum: md5sum sum
NOBIN = getconf

CAT = $(STDCAT|"cat")
CMP = $(STDCMP|"cmp") 2>/dev/null
CP = $(STDCP|"cp")
LN = $(STDLN|"ln")
RM = $(STDRM|"rm")

BUILTINS :COMMAND: cmdlist.h
	$(SED) -e '/^CMDLIST(.*,.*)$/!d' -e 's/CMDLIST(\(.*\),.*)/\1/' $(NOBIN:C,.*,-e '/^&$/d',) $(LINKS:N!=*[:]:C,.*,-e '/^&$/d',) $(*)

.INIT : .builtin

.builtin : .MAKE .VIRTUAL .FORCE
	local T I
	if T = "$(NOBIN:D=$(BINDIR):B:S:T=F)"
		{ $(RM) $(RMFLAGS) $(T) }
	end
	:ALL: $(BUILTINS)
	T = -lcmd
	if "$(PACKAGE_OPTIONS:N=optimize-space)" && "$(T:T=F)" == "-lcmd"
		T := $(BUILTINS:O=1)
		$(T) :: CMD_BUILTIN=1 CMD_DYNAMIC=1 builtin.c -lcmd -ldll
		for I $(BUILTINS:O>1)
			$(I) :LINK: $(T)
		end
		for I $(LINKS)
			if I != "*:"
				$(I) :LINK: $(T)
			end
		end
	else
		$(BINDIR) :INSTALLDIR: $(BUILTINS)
		$(BUILTINS) : .COMMAND $(&$("%.c":T=SM%)) CMD_BUILTIN=b_$$(<:B) builtin.c $(*$("%.c":T=SM%))
			$(CAT) $(*:N=*builtin.c) > $(<).c
			$(RM) $(RMFLAGS) $(<)
			$(@$("%.c":T=SM%):/.*builtin.c$/$(<).c/)
			$(RM) $(RMFLAGS) $(<).c
		for I $(LINKS)
			if I == "*:"
				T := $(I:/.$//)
			else
				$(I) :LINK: $(T)
			end
		end
	end

":BUILTIN:" : .MAKE .OPERATOR
	local T P
	for T $(>)
		if T == "*=*"
			$(P).o : $(T)
		elif T == "[-+]l*"
			$(P) : $(T)
		else
			eval
			$(T) :: CMD_STANDALONE=b_$(T) $(T).c
			end
			P := $(T)
		end
	end

:BUILTIN: dlls LICENSE=since=2002,author=gsf -ldll look mime \
		pty LICENSE=since=2001,author=gsf+dgk -lutil \
		nl LICENSE=since=2003,author=dgk asa pr \
		strings uudecode -luu uuencode -luu what who

dontcare -lutil

:: PROMO.mm RELEASE

:TEST: asa cat chmod chown cksum cmp cp cut date expr fmt grep head \
	join ln look ls mkdir mkfifo mktemp mv nl od paste readlink \
	realpath rm tail tr uniq uuencode wc xargs
