/*
 * zlib makefile
 */

:PACKAGE: ast

LICENSE = zlib,since=1995,author="Jean-loup Gailly,Mark Adler"

ID = z
VERSION = 1.2

CCFLAGS = $(CC.OPTIMIZE) $(CC.DLL)

$(ID) $(VERSION) :LIBRARY: zlib.3 zlib.h zconf.h zutil.h \
	crc32.h deflate.h inffast.h inffixed.h inflate.h inftrees.h trees.h \
	adler32.c compress.c crc32.c deflate.c gzio.c infback.c inffast.c \
	inflate.c inftrees.c trees.c uncompr.c zutil.c \
	sfdcgzip.c sfdclzw.c

$(INCLUDEDIR) :INSTALLPROTO: zlib.h zconf.h sfdcgzip.h

:: RELEASE README INDEX ChangeLog algorithm.doc

:INSTALLDIR: example minigzip

example :: example.c -lz

minigzip :: minigzip.c -lz

test : example minigzip
	$(*:N=*example) > /dev/null || echo libz example test FAILED
	case `echo 'hello, hello!' | $(*:N=*minigzip) | $(*:N=*minigzip) -d; echo $?` in
	'hello, hello!$("\n")0')
		;;
	*)	echo libz minigzip test FAILED
		;;
	esac
	rm -f foo.gz
