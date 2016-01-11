########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1999-2011 AT&T Intellectual Property          #
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
# addition date.dat test generator
#

now=2008-05-01+01:02:03
regress=$1

if	[[ $regress ]]
then	printf $'%s\t\t\t\t\t%s\t%s\n' "SET" "NOW" "$now"
fi

while	read d
do	if	[[ $d ]]
	then	# printf $'%T  %s\n' "$d" "$d"
		if	[[ $regress ]]
		then	width=${#d}
			if	(( width < 8 ))
			then	sep=$'\t\t\t\t\t'
			elif	(( width < 16 ))
			then	sep=$'\t\t\t\t'
			elif	(( width < 24 ))
			then	sep=$'\t\t\t'
			elif	(( width < 32 ))
			then	sep=$'\t\t'
			else	sep=$'\t'
			fi
			if	[[ $d == +([a-z])day ]]
			then	printf $'%s%sNULL\t%s\n' "$d" "$sep" "$(date -s "$now $d" | tail -1)"
			else	printf $'%s%sNULL\t%s\n' "$d" "$sep" "$(date -s "$now" "$d" | tail -1)"
			fi
		else	printf $'%s %s\n' "$(date -s "$now" "$d" | tail -1)" "$d"
		fi
	else	printf $'\n'
	fi
done <<!

now

sunday
monday
tuesday
wednesday
thursday
friday
saturday

this month
last month
next month

this month 1st monday
this month first monday
1st monday may 2008
first monday may 2008

this month 2nd monday
this month second monday
2nd monday may 2008
second monday may 2008

this month 3rd monday
this month third monday
3rd monday may 2008
third monday may 2008

this month 4th monday
this month fourth monday
4th monday may 2008
fourth monday may 2008

this month 5th monday
this month fifth monday
5th monday may 2008
fifth monday may 2008

this month 6th monday
this month sixth monday
6th monday may 2008
sixth monday may 2008

this month nth monday
this month final monday
nth monday may 2008
final monday may 2008

last month first monday
last month 1st monday
last month second monday
last month 2nd monday
last month third monday
last month 3rd monday
last month fourth monday
last month 4th monday
last month final monday
last month nth monday

next month first monday
next month 1st monday
next month second monday
next month 2nd monday
next month third monday
next month 3rd monday
next month fourth monday
next month 4th monday
next month final monday
next month nth monday

!
