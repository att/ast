########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1990-2011 AT&T Intellectual Property          #
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
: generate list of file servers
#
# genshare [options]
#
# @(#)genshare (AT&T Research) 2002-10-02
#

dir=.
rsh=rsh
while	:
do	case $# in
	0)	break ;;
	esac
	case $1 in
	-[dr])	case $# in
		1)	set -- '-?'
			break
			;;
		esac
		case $1 in
		-d)	dir=$2 ;;
		-r)	rsh=$2 ;;
		esac
		shift
		;;
	'-?')	break
		;;
	[-+]*)	echo "$0: $1: invalid option" >&2
		set -- '-?'
		break
		;;
	*)	break
		;;
	esac
	shift
done
case $1 in
?*)	echo "Usage: $0 [-d share-dir] [-r rsh-path]" >&2; exit 1 ;;
esac
if	test ! -d $dir
then	echo "$0: $dir: invalid directory" >&2; exit 1
fi
s="`df 2>/dev/null | sed -e 's/^[^(]*(//' -e '/^[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]*:/!d' -e 's/:.*//' -e '/^.$/d' | sort -u`"
case $s in
"")	s=`hostname 2>/dev/null || uname -n 2>/dev/null || cat /etc/whoami 2>/dev/null || echo local 2>/dev/null`
	;;
*)	p=`df $dir/. 2>/dev/null | sed -e 's/^[^(]*(//' -e '/^[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]*:/!d' -e 's/:.*//' -e '/^.$/d'`
	case $p in
	"")	p=`hostname 2>/dev/null || uname -n 2>/dev/null || cat /etc/whoami 2>/dev/null || echo local 2>/dev/null` ;;
	esac
	t=$s
	s=$p
	for h in $t
	do	case $s in
		*$h*)	;;
		*)	s="$s
$h" ;;
		esac
	done
	t=$s
	s=`(echo "$t"; for h in $t
	do	$rsh $h df '2>/dev/null' | sed -e 's/^[^(]*(//' -e '/^[abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_]*:/!d' -e 's/:.*//'
	done) | sort | uniq -c | sort -bnr | sed -e 's/^[ 	]*[0-9]*[ 	]*//'`
	;;
esac
echo '#'
echo '# local file servers'
echo '#'
echo "$s"
