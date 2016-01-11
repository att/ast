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
: generate local host info
#
# genlocal [options] [host ...]
#
# @(#)genlocal (AT&T Research) 2002-10-02
#
# NOTE: this only works for hosts the caller can reach by rsh/ssh
#

case $RANDOM in
$RANDOM)	exec ksh $0 "$@"; echo "$0: ksh required" >&2; exit 1 ;;
esac
integer cluster=5
dir=.
file=
heading=1
list=
rsh=rsh
timeout=90
verbose=:
while	:
do	case $# in
	0)	break ;;
	esac
	case $1 in
	-[frt])case $# in
		1)	set -- '-?'
			break
			;;
		esac
		case $1 in
		-f)	file=$2 ;;
		-r)	rsh=$2 ;;
		-t)	timeout=$2 ;;
		esac
		shift
		;;
	-h)	heading=
		;;
	-n)	list=1
		;;
	-v)	verbose='print -u2'
		;;
	'-?')	break
		;;
	[-+]*)	print -u2 "$0: $1: invalid option"
		set -- '-?'
		break
		;;
	*)	break
		;;
	esac
	shift
done
case $1 in
'-?')	print -u2 "Usage: $0 [-hnv] [-f share-file] [-r rsh-path] [-t timeout] [host ...]"; exit 1 ;;
esac
case $file in
"")	ifs=${IFS-'
	 '}
	IFS=:
	set "" $PATH "--" "$@"
	IFS=$ifs
	f=lib/cs/share
	while	:
	do	shift
		case $1 in
		"")	continue ;;
		--)	break ;;
		esac
		case $file in
		"")	test -f ${1%bin}$f && file=${1%bin}$f ;;
		esac
	done
	shift
	case $file in
	"")	print -u2 $0: $f: not found; exit 1 ;;
	esac
	;;
*)	if	test ! -f $file
	then	print -u2 "$0: $file: cannot read share file"; exit 1
	fi
	;;
esac
print -u2 "$0: warning: $rsh may hang on some hosts -- monitor the progress with the -v option or ps $$"
server=
for host in $(egrep -v '^#' $file)
do	case $server in
	"")	server=$host ;;
	*)	server="$server|$host" ;;
	esac
done
hostname=$(package host name)
hostname=${hostname%%.*}
case $# in
0)	domain=$(
		{
			cat /etc/hosts
			ypcat hosts
		} 2>/dev/null |
		egrep "[^#].*[ 	]$hostname(\$|[. 	])" |
		sed -e '1!d' -e 's/^[^ 	]*//' -e 's/[^.]*.//' -e 's/[ 	].*//'
	)
	set -- $(
		{
			print "$hostname"
			egrep -v '^#' $file
			ruptime | fgrep -v 'no hosts'
			{
				for i in /etc/resolv.conf /usr/etc/resolv.conf
				do	if	test -f $i
					then	echo "ls -t A $(sed -e '/^domain/!d' -e 's/.*[ ][ 	]*//' $i)" | nslookup
						break
					fi
				done
				cat /etc/hosts
				ypcat hosts
			} |
			fgrep ".$domain" |
			awk '{print $2}'
		} 2>/dev/null |
		sed -e 's/[. 	].*//' -e '/^[a-zA-Z].*[a-zA-Z0-9]$/!d' |
		sort -u
	)
	case $list in
	?*)	print domain=$domain
		;;
	esac
	;;
esac
case $list in
?*)	print hosts='"'$*'"'; exit ;;
esac
hosts=
for host
do	case $host in
	local|localhost)	host=$hostname ;;
	esac
	case " $hosts " in
	*" $host "*)	continue ;;
	esac
	hosts="$hosts $host"
	$verbose -n "$host "
	case $host in
	$hostname)
		package host name type cpu rating
		;;
	*)	if	ping -c 1 -w 4 $host >/dev/null 2>&1
		then	$rsh $host bin/package host name type cpu rating &
			info=$!
			{
				sleep $timeout
				kill -9 $info
			} &
			time=$!
			wait $info
			kill -9 $time
		fi
		;;
	esac 2>/dev/null
done | while read host type cpu rating
do	$verbose type=$type cpu=$cpu rating=$rating
	host=${host%%.*}
	eval "case '$host' in
	$server)	idle= ;;
	*)		idle=15m ;;
	esac"
	print $host"	"type=$type"	"${rating}${idle:+"	idle=$idle"}${cpu:+"	cpu=$cpu"}
done | sort -b +2n -3 | {
	integer v c d i j k n
	((n=0))
	while read host type rate attr
	do	((n=n+1))
		h[n]=$host
		t[n]=$type
		v[n]=$rate
		a[n]=$attr
	done
	c=1
	while	((c))
	do	((c=0))
		((i=1))
		while	((i<n))
		do	((d=v[i+1]-v[i]))
			if	((d>1&&d<cluster))
			then	((d=d/2))
				((v[i]=v[i]+d))
				((v[i+1]=v[i+1]-d))
				c=1
			fi
			((i=i+1))
		done
	done
	((i=1))
	while	((i<=n))
	do	((c=v[i]))
		((j=i+1))
		((k=1))
		while	((j<=n))
		do	if	(((v[j]-v[j-1])>1||(v[j]-v[i])>cluster))
			then	break
			fi
			((c=c+v[j]))
			((k=k+1))
			((j=j+1))
		done
		((c=c/k))
		((d=c%10))
		((c=c-d))
		if	((d>6))
		then	((c=c+10))
		elif	((d>3))
		then	((c=c+5))
		fi
		while	((i<j))
		do	((v[i]=c))
			((i=i+1))
		done
	done
	((i=1))
	while	((i<=n))
	do	rate=${v[i]}
		print ${h[i]} ${t[i]} rating=${v[i]}"	${a[i]}"
		((i=i+1))
	done
} | sort -b +1 -2 +2.7n -3 +0 -1 | while read host type attr
do	case $heading in
	1)	print '#'
		print '# local host attributes'
		print '#'
		print
		print 'local		busy=2m		pool=9'
		print
		;;
	''|$type)
		;;
	*)	print
		;;
	esac
	heading=$type
	case $host in
	????????*)
		tab1="	"
		;;
	*)	tab1="		"
		;;
	esac
	case $type in
	????????*)
		tab2="	"
		;;
	*)	tab2="		"
		;;
	esac
	print $host"$tab1"$type"$tab2$attr"
done
