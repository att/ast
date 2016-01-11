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
command=delta2patch

USAGE=$'
[-?
@(#)$Id: delta2patch (AT&T Research) 2007-12-11 $
]
'$USAGE_LICENSE$'
[+NAME?delta2patch - generate patch script from pax delta+base archives]
[+DESCRIPTION?\bdelta2patch\b generates a \bpatch\b(1) \adiff -u\a script
	given a \bpax\b(1) delta and base source archive to convert
	files extracted from the base archive to be equivalent to files
	extracted from the delta archive with respect to the base archive.]

delta base

[+SEE ALSO?\bpackage\b(1), \bpax\b(1)]
'

function usage
{
	OPTIND=0
	getopts -a $command "$USAGE" OPT '--??long'
	exit 2
}

while	getopts -a $command "$USAGE" OPT
do	case $OPT in
	*)	usage ;;
	esac
done

if	(( $# != 2 ))
then	usage
fi
delta=$1
base=$2
changes=$(pax --nosummary -f $delta 2>/dev/null | sed -e '/^\(update\|create\) /!d' -e 's/^[^ ]* //')
tmp=/tmp/d2p-$$
trap "rm -rf $tmp" 0
mkdir $tmp $tmp/old $tmp/new || exit
pax --nosummary -rf $base -s ",.*,$tmp/old/&," $changes
pax --nosummary -rf $delta -z $base -s ",.*,$tmp/new/&," $changes
diff -r -N -u $tmp/old $tmp/new | sed -e "s,$tmp/new/,,g" -e "s,$tmp/old/,,g" -e $'s/^--- \\([^\t]*\\)/&.orig/' -e '/^diff /s/ [^ ]*$/.orig&/'
