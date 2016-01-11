########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1984-2011 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                    David Korn <dgkorn@gmail.com>                     #
#                             Pat Sullivan                             #
#                                                                      #
########################################################################
: include OPTIONS
eval $1
shift
. $1
for i in ACCT BRACEPAT ECHOPRINT ESH ESHPLUS FLOAT FS_3D IODELAY JOBS \
	MULTIBYTE NEWTEST OLDTERMIO OLDTEST P_UID PDU POSIX RDS RAWONLY \
	SEVENBIT SUID_EXEC TIMEOUT VSH LDYNAMIC
do	: This could be done with eval, but eval broken in some shells
	j=0
	case $i in
	ACCT)		j=$ACCT;;
	BRACEPAT)	j=$BRACEPAT;;
	ECHOPRINT)	j=$ECHOPRINT;;
	ESH)		j=$ESH;;
	ESHPLUS)	j=$ESHPLUS;;
	FLOAT)		j=$FLOAT;;
	FS_3D)		j=$FS_3D;;
	IODELAY)	j=$IODELAY;;
	JOBS)		j=$JOBS;;
	LDYNAMIC)	j=$LDYNAMIC;;
	MULTIBYTE)	j=$MULTIBYTE;;
	NEWTEST)	j=$NEWTEST;;
	OLDTERMIO)	echo "#include <sys/termios.h>" > $tmp.c
			echo "#include <sys/termio.h>" >> $tmp.c
			if	$CC -E $tmp.c > /dev/null 2>&1
			then	j=$OLDTERMIO
			fi ;;
	OLDTEST)	j=$OLDTEST;;
	P_UID)		j=$P_UID;;
	PDU)		j=$PDU
			if	test 0 -ne "$j" && test -f /usr/pdu/bin/pwd
			then	echo "#define PDUBIN	1"
			fi;;
	POSIX)		j=$POSIX;;
	RAWONLY)	j=$RAWONLY;;
	RDS)		j=$RDS;;
	SEVENBIT)	j=$SEVENBIT;;
	SUID_EXEC)	j=$SUID_EXEC;;
	TIMEOUT)	j=$TIMEOUT;;
	VSH)		j=$VSH;;
	esac
	case $j in
	0|"")	;;
	*)	echo "#define $i	$j" ;;
	esac
done
