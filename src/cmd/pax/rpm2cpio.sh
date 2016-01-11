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
: rpm2cpio wrapper for pax

command=rpm2cpio
case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: rpm2cpio (AT&T Labs Research) 1999-12-25 $
]
'$USAGE_LICENSE$'
[+NAME?rpm2cpio - convert rpm file to cpio file]
[+DESCRIPTION?\brpm2cpio\b converts the input \brpm\b(1) \afile\a, or
	the standard input if \afile\a is omitted, to a \bcpio\b(1)
	file on the standard output.]
[+?This command is provided for interface compatibility; use \bpax\b(1) to
	read \brpm\b files directly.]
[n:show?Show the underlying \bpax\b(1) command but do not execute.]
[v:verbose?List the package file members as they are converted.]

[ file ]

[+SEE ALSO?\bcpio\b(1), \bpax\b(1), \brpm\b(1)]
'
	;;
*)	ARGV0=""
	USAGE="nv [ file ]"
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

show=
options=
while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	n)	show="print --" ;;
	v)	options="$options -v" ;;
	*)	echo "Usage: $command [-nv] [ file ]" >&2
		exit 2
		;;
	esac
done
shift $OPTIND-1
case $# in
0)	input=- ;;
1)	input=$1 ;;
*)	usage ;;
esac
$show pax $options -r -f "$input" -w -x cpio
