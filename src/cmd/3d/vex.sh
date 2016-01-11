########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1989-2011 AT&T Intellectual Property          #
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
#                 Glenn Fowler <gsf@research.att.com>                  #
#                  David Korn <dgk@research.att.com>                   #
#                   Eduardo Krell <ekrell@adexus.cl>                   #
#                                                                      #
########################################################################
: vex skip expand command ...
# vpath expand args matching pattern
# @(#)vex (AT&T Bell Laboratories) 04/01/93
case $# in
[012])	print -u2 "Usage: $0 skip expand command ..."; exit 1 ;;
esac
skip=$1
shift
expand=$1
shift
command=$1
shift
integer argc=0 noexpand=0
for arg
do	if	((noexpand))
	then	noexpand=0
	else	case $arg in
		-[$skip])
			noexpand=1
			;;
		-*)	;;
		$expand)
			x=$(vpath "$arg" 2>/dev/null)
			case $x in
			?*)	arg=${x#$PWD/} ;;
			esac
			;;
		esac
	fi
	argv[argc]=$arg
	((argc+=1))
done
"$command" "${argv[@]}"
