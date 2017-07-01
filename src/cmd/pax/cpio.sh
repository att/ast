########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1987-2011 AT&T Intellectual Property          #
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
#               Glenn Fowler <glenn.s.fowler@gmail.com>                #
#                                                                      #
########################################################################
:
# @(#)cpio.sh (AT&T Labs Research) 1990-08-11
#
# cpio -> pax interface script
#

command=cpio
usage="
Usage: $command -o[acvBV] [-C size] [-M mesg] [-O file | >file ] <list
       $command -i[bcdfkmrtsuvBSV6] [-I file | <file] [pattern ...]
       $command -p[adlmuvV] directory"

OPTSTR='abcdfiklmoprstuvBSV6C:[size]H:[format]M:[message]O:[file]?D [ pattern | directory ]'

options="-d"
blocksize=1b
exec=eval
format=binary
list=""
logphys=-P
mode=""
d_default="-o nomkdir"
m_default="-pm"
u_default="-u"
r_ok="1"
w_ok="1"
p_ok="1"

while	getopts -a $command "$OPTSTR" OPT
do	case $OPT in
	'D')	case $exec in
		eval)	exec=print ;;
		*)	exec="eval args" ;;
		esac
		;;
	[bsS6]) ;;
	[klvV])	options="$options -$OPT" ;;
	a)	r_ok="" options="$options -p" ;;
	c)	format=cpio ;;
	d)	w_ok="" d_default="" ;;
	f)	w_ok="" p_ok="" options="$options -c" ;;
	i)	w_ok="" p_ok=""
		case $mode in
		'')	mode=-r ;;
		-r)	;;
		*)	mode=x ;;
		esac
		;;
	m)	w_ok="" m_default="" ;;
	o)	r_ok="" p_ok="" u_default=""
		case $mode in
		'')	mode=-w ;;
		-w)	;;
		*)	mode=x ;;
		esac
		;;
	p)	r_ok="" w_ok=""
		case $mode in
		'')	mode=-rw ;;
		-rw)	;;
		*)	mode=x ;;
		esac
		;;
	r)	w_ok="" p_ok="" options="$options -i" ;;
	t)	w_ok="" p_ok="" list="1" ;;
	u)	w_ok="" u_default="" ;;
	B)	blocksize=5k ;;
	L)	logphys=-L ;;
	C)	case $OPTARG in
		*[0-9])	blocksize=${OPTARG}c ;;
		*)	blocksize=${OPTARG} ;;
		esac
		;;
	H)	case $OPTARG in
		asc|ASC)	format=asc ;;
		crc|CRC)	format=aschk ;;
		odc|ODC)	format=cpio ;;
		tar|TAR)	format=tar ;;
		ustar|USTAR)	format=ustar ;;
		*)		print -u2 "$command: $OPTARG: formats are {asc,crc,odc,tar,star}"; exit 1 ;;
		esac
		;;
	I)	w_ok="" p_ok="" options="$options -f '$OPTARG'" ;;
	O)	r_ok="" p_ok="" options="$options -f '$OPTARG'" ;;
	M)	options="$options -o eom=\"'$OPTARG'\"" ;;
	esac
done
shift $(($OPTIND-1))
case $mode in
-r)	case $r_ok in
	"")	print -u2 "$command: options inconsistent with archive read"
		exit 2
		;;
	esac
	options="$options -b $blocksize"
	;;
-w)	case $w_ok in
	"")	print -u2 "$command: options inconsistent with archive write"
		exit 2
		;;
	esac
	case $# in
	0)	;;
	*)	print "$command: arguments not expected"
		exit 2
		;;
	esac
	options="$options -x $format -b $blocksize"
	d_default="" m_default="" u_default=""
	;;
-rw)	case $p_ok in
	"")	print -u2 "$command: options inconsistent with file pass"
		exit 2
		;;
	esac
	case $# in
	1)	;;
	*)	print -u2 "$command: a single directory argument is expected$usage"
		exit 2
		;;
	esac
	;;
'')	print -u2 "$command: one of -i, -o, -p must be specified$usage"
	exit 2
	;;
*)	print -u2 "$command: only one of -i, -o, -p may be specified$usage"
	exit 2
	;;
esac

case $list in
"1")	mode="" d_default="" m_default="" u_default="" ;;
esac

case $exec in
eval) $exec pax $mode $logphys $options $d_default $m_default $u_default '"$@"' ;;
*) $exec pax $mode $logphys $options $d_default $m_default $u_default "$@" ;;
esac
