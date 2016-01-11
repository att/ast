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
:
# opaque file ...
# handles old (pre-92) and new style opaque
# @(#)opaque (AT&T Bell Laboratories) 10/11/95

function err_exit
{
	print -u2 "$command: $@"
	exit 1
}

function usage
{
	print -u2 "Usage: $command file ..."
	exit 2
}

command=${0##*/}
case $1 in
--)	shift
	break
	;;
""|-\?)	usage
	;;
-*)	print -u2 -- "$1: invalid option"
	usage
	;;
esac

# the rest is in 2d

2d + 2>/dev/null || { { : > xxx && rm xxx && 2d + ;} || exit 1 ;}
n=$(umask)

for i
do	if	[[ -f $i ]]
	then	[[ $i -ef .../... ]] || print -u2 "$command: $i: file exists"
	else	case $i in
		*/*)	dir=${i%/*} file=${i##*/} ;;
		*)	dir=. file=$i ;;
		esac
		[[ $dir -ef . ]] || cd "$dir" || err_exit "$dir: cd failed"
		if	[[ ! -f .../... ]]
		then	if	[[ ! -d ... ]]
			then	if	mkdir ...
				then	chgrp 3d ... 2>/dev/null && chmod g+s ...
					umask 777
					> .../... || err_exit "$i: cannot opaque"
					umask ${n#8#}
				elif	[[ ! -f ... ]]
				then	err_exit "...: cannot mkdir"
				elif	[[ -r ... || -w ... || -x ... || -s ... ]] 
				then	err_exit "...: obsolescent opaque must be 0 mode file"
				else	mkdir ...3d || err_exit "...3d: cannot mkdir"
					if	mv ... ...3d
					then	if	mv ...3d ...
						then	:
						else	rmdir ...
							mv ...3d ...
							err_exit "...: cannot rename obsolescent opaque"
						fi
					else	rmdir ...3d
						err_exit "...: cannot rename obsolescent opaque"
					fi
					chgrp 3d ... 2>/dev/null && chmod g+s ...
				fi
			else	umask 777
				> .../... || err_exit "$i: cannot opaque"
				umask ${n#8#}
			fi
		fi
		ln .../... "$file"
		[[ $dir -ef . ]] || cd ~- || err_exit "$OLDPWD: cannot restore directory"
	fi
done
