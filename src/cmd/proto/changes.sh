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
command=changes

case `(getopts '[-][123:xyz]' opt --xyz; echo 0$opt) 2>/dev/null` in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: changes (AT&T Research) 2010-01-20 $
]
'$USAGE_LICENSE$'
[+NAME?changes - list changed $PACKAGEROOT package source files]
[+DESCRIPTION?\bchanges\b lists $PACKAGEROOT relative package source
    files that changed between \afrom-date\a between \ato-date\a. The
    default \afrom-date\a is the modify time of \b$PACKAGEROOT/README\b.
    The default \ato-date\a is \bnow\b. The changed files are listed by
    \bls\b(1) with the optional \bls-option\b operands.]
[p:package?List source files for \apackage\a only. Multiple
    \b--package\b options may be specified. By default source files for all
    packages in \b$PACKAGEROOT/lib/package\b are checked.]:[package]

[ from-date [ to-date ] ] [ -- ls-option ... ]

[+SEE ALSO?\btw\b(1), \bls\b(1)]
'
	;;
*)	ARGV0=""
	USAGE=" table ..."
	;;
esac

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

packages=
while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	p)	if	[[ $packages ]]
		then	packages="$packages|$OPTARG"
		else	packages=$OPTARG
		fi
		;;
	*)	usage
		;;
	esac
done
shift $OPTIND-1

if	[[ ! $PACKAGEROOT ]]
then	print -u2 $command: \$PACKAGEROOT: package root dir not exported
	exit 1
fi
cd $PACKAGEROOT || exit

if	[[ $packages ]]
then	packages="@(?(*-)($packages))"
else	packages='*'
fi
packages=$packages.pkg

if	[[ ! $1 || $1 == -* ]]
then	if	[[ ! -f README ]]
	then	print -u2 $command: a date must be specified
		exit 1
	fi
	from=$(date -f %K -m README)
else 	from=$1
	shift
fi
from="&& mtime >= '$from'"
if	[[ $1 && $1 != -* ]]
then	to=" && mtime <= '$1'"
	shift
else 	to=
fi
ls_options=$*

set -- $(eval print lib/package/$packages)
if	[[ ! -f $1 ]]
then	print -u2 $command: lib/package: no packages
	exit 1
fi
packages=$*
components=
sep=
for pkg
do	while	read line
	do	if	[[ $line == *:PACKAGE:* ]]
		then	set -- $line
			while	(( $# )) && [[ $1 != :PACKAGE: ]]
			do	shift
			done
			if	(( $# ))
			then	shift
				for p
				do	p=${p#'$(PACKAGEROOT)/'}
					if	[[ $p != *'$('* ]]
					then	components=$components$sep$p
						sep=' '
					fi
				done
				break
			fi
		fi
	done < $pkg
done
if	[[ ! $components ]]
then	print -u2 $command: ${packages//' '/,}: no components
	exit 1
fi
tw $(eval ls -d $packages src/@(lib|cmd)/@(${components//' '/'|'}) |
     sed 's/^/-d /') \
     -e "type != DIR$from$to" \
     ls $ls_options
