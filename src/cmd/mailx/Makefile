:PACKAGE: ast --clobber=!(mailx)

LICENSE = bsd,since=1978,author='Kurt Shoens (UCB)+gsf',corporation='The Regents of the University of California and AT&T'

mailx :: README RELEASE mailx.1 \
	mailx.h local.h \
	data.c cmd1.c cmd2.c cmd3.c collect.c edit.c \
	fio.c head.c local.c lex.c list.c main.c misc.c \
	names.c proc.c quit.c send.c spam.c tty.c vars.c \
	imap.c smtp.c -lcs

:: port/cdt.h port/dthdr.h port/mime.h port/mimelib.h \
	port/dtclose.c port/dtdisc.c port/dtextract.c port/dtflatten.c \
	port/dthash.c port/dtlist.c port/dtmethod.c port/dtopen.c \
	port/dtrenew.c port/dtrestore.c port/dtsize.c port/dtstat.c \
	port/dtstkq.c port/dtstrhash.c port/dttree.c port/dtview.c \
	port/dtwalk.c port/mime.c port/mimetype.c

:MSGFUN: note _ast_printf
