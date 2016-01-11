########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1989-2012 AT&T Intellectual Property          #
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
#
# seq.sh
# Written by David Korn
# AT&T Labs
# Sat May  5 00:32:40 EDT 2007
#
case $(getopts '[-]' opt "--???man" 2>&1) in
version=[0-9]*)
	usage=$'\n[-?\n@(#)$Id: seq (AT&T Labs Research) 2012-04-14 $\n]
	'$USAGE_LICENSE$'
	[+NAME?seq - print a sequence of numbers]
	[+DESCRIPTION?\bseq\b writes the numbers from \afirst\a to \alast\a
		in steps of increments.  If \afirst\a or \aincr\a is omitted,
		it defaults to 1.  An omitted \aincr\a defaults to 1 even
		when \alast\a is smaller than \afirst\a.]
	[+?\afirst\a \aincr\a,  and \alast\a are interpreted as floating
		point values.  \aincr\a is usually positive if \afirst\a  is
		smaller than \alast\a, and \aincr\a is usually negative
		if \afirst\a is greater than \alast\a.  When given, the
		\aformat\a argument must contain exactly one of the
		printf-style, floating point output formats \b%e\b, \b%f\b,
		\b%g\b.]
	[f:format]:[format:=%g?use printf style floating point \aformat\a.]
	[s:separator]:[string:=\\n?use \astring\a to separate numbers.]
	[w:equal-width?equalize width by padding with leading zeroes.]

	[ first  [ incr ] ] last

	[+EXIT STATUS?]{
       		 [+0?Success.]
       		 [+>0?An error occurred.]
	}
	[+SEE ALSO?\bksh\b(1), \bprintf\b(3)]
	'
	;;
*)
	usage=''
	;;
esac

function err_exit
{
	print -ru2 -- "$command: $@"
	exit 1
}

function format # first second next_to_last last
{
	typeset i
	integer width=0 precision=0 n
	for i
	do	n=${#i}
		(( n > width )) && width=$n
		[[ $i != *.* ]] && continue
		i=${i##*.}
		n=${#i}
		(( n > precision )) && precision=$n
	done
	print -r -- "%0$width.${precision}f"
}

command=${0##*/}
width= sep=$'\n' fmt= end=
float first=1 incr=1 last n sign=1 epsilon
while getopts "$usage" var
do	case $var in
	f)
		fmt=$OPTARG
		[[ $fmt == *%*([^[:space:][:alpha:]])[efg]* ]] || err_exit "$fmt: invalid format string"
		;;
	s)
		sep=$OPTARG;;
	w)
		width=1;;
	esac
done
shift $((OPTIND-1))
case $# in
1)	last=$1;;
2)	first=$1 last=$2;;
3)	first=$1 incr=$2 last=$3;;
0)	err_exit "too few arguments";;
*)	err_exit "too many arguments";;
esac
if	[[ $width ]]
then	[[ $fmt ]] && err_exit "format string may not be specified when printing equal width strings"
	(( n = (last - first) / incr ))
	if	(( abs(floor(n) - n) <= 1e-12 ))
	then	end=$last
	fi
	fmt=$(format $first $((first+incr)) $((last-incr)) $end)
elif	[[ ! $fmt ]]
then	fmt=%g
fi
(( incr<0)) && sign=-1
(( epsilon = sign*incr*1e-12 ))
(( last += sign*epsilon ))
(( last *= sign ))
s=
for ((n=first; (sign*n) <= last; n+= incr ))
do	printf "%s$fmt" "$s" $(( abs(n) > epsilon ? n : 0 ))
	s=$sep
done
[[ $s ]] && print
exit 0
