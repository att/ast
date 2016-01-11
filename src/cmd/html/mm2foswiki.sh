########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1996-2012 AT&T Intellectual Property          #
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
: mm2foswiki - convert mm/man subset to foswiki markups

# it keeps going and going ...
#
# \h'0*\w"URL"'HOT-TEXT\h'0'	link goto with alternate url
# \h'0*1'HOT-TEXT\h'0'		link goto
#
# .xx link="URL\tHOT-TEXT"	link goto with url request
# .xx link="HOT-TEXT"		link goto request
# .xx ref="URL\tMIME-TYPE"	head link hint

command=mm2foswiki
version='mm2foswiki (AT&T Research) 2012-08-15' # NOTE: repeated in USAGE
LC_NUMERIC=C
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: mm2foswiki (at&t research) 2012-08-15 $
]
'$USAGE_LICENSE$'
[+NAME?mm2foswiki - convert mm/man subset to foswiki markups]
[+DESCRIPTION?\bmm2foswiki\b is a \bsed\b(1)/\bksh\b(1) script (yes!) that
	converts input \bmm\b(1) or \bman\b(1) documents to a \bfoswiki\b markups
	document on the standard output. If \afile\a is omitted then the
	standard input is read.  \adir\a operands and directory components
	of \afile\a operands are added to the included file search list.]
[f:frame?Ignored for compatibility with \bmm2html\b(1).]:[name]
[g:global-index?Ignored for compatibility with \bmm2html\b(1).html\b for framed HTML.]
[h:html?Read html options from \afile\a. Unknown options are silently
    ignored. See the \b.xx\b request below for a description of the
    options. The file pathname may be followed by URL style \aname=value\a
    pairs that are evaluated as if they came from
    \afile.\a]:[file[??name=value;...]]]
[l:license?Read license identification options from \afile\a. Unknown
    options are silently ignored. See the \b.xx\b request below for a
    description of the options. The file pathname may be followed by URL
    style \aname=value\a pairs that are evaluated as if they came from
    \afile\a.]:[file[??name=value;...]]]
[o:option?Sets a space or \b,\b separated list of \b--license\b
    options. Option values with embedded spaces must be
    quoted.]:[[no]]name=value]
[t:top?Ignored for compatibility with \bmm2html\b(1).]
[x:index?Ignored for compatibility with \bmm2html\b(1).]

[ file ... ]

[+SEE ALSO?\bmm2html\b(1), \bmm2twiki\b(1)]
'
	;;
*)	ARGV0=""
	USAGE=' [ file ... ]'
	;;
esac

set -o noglob

integer count row n s ndirs=0 nfiles=0
integer fd=0 head=2 line=0 lists=0 nest=0 peek=0 pp=0 so=0 soff=4
integer labels=0 reference=1 ce=0 nf=0 augment=0 tbl_ns=0 tbl_no=1 tbl_fd=1
integer bold colon
typeset -Z2 page=01
typeset -u upper
typeset -x -l OP
typeset -x -A ds map nr outline
typeset cond dirs files fg frame label list li lc prev text title type
typeset license html meta nl mm index authors vg references
typeset LI="   "
typeset mm_AF mm_AF_cur mm_AF_old mm_AU tt

nl=$'\n'

html=(
	H1=(
		align=center
	)
	home=(
		href=
	)
	index=(
		left=
		top=
	)
	logo=(
		src=
	)
	width=96%
)

code=0
figure=1
file=
font=
frame=
framebody=
framelink=
framerefs=
hp=
ifs=${IFS-'
	'}
inch="     "
indent=
indent_prev=
macros=
pd="$nl"
pm=
primary=".BL|.LI|.IX"
top=
vg_ps=20

function options
{
	typeset i o q v
	IFS="${IFS},"
	set $OPTARG
	IFS=$ifs
	for i
	do	case $q in
		'')	o=${i%%=*}
			v=${i#*=}
			case $v in
			\'*)	q=\'; v=${v#?}; continue ;;
			\"*)	q=\"; v=${v#?}; continue ;;
			esac
			;;
		*)	v="$v $i"
			case $i in
			*$q)	v=${v%?} ;;
			*)	continue ;;
			esac
			;;
		esac
		case $o in
		no*)	o=${o#no}
			unset $o
			;;
		*)	case $v in
			"")	v=1 ;;
			esac
			case $o in
			*.*|[ABCDEFGHIJKLMNOPQRSTUVWXYZ]*)
				eval $o="'$v'"
				;;
			*)	eval license.$o="'$v'"
				;;
			esac
			;;
		esac
		set[$o]=1
	done
}

if	[[ -f $HOME/.2html ]]
then	. $HOME/.2html
fi

usage()
{
	OPTIND=0
	getopts $ARGV0 "$USAGE" OPT '-?'
	exit 2
}

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	f)	: ignored :
		;;
	g)	: ignored :
		;;
	h)	case $OPTARG in
		*\?*)	. ${OPTARG%%\?*} || exit 1
			eval "html+=( ${OPTARG#*\?} )"
			;;
		*)	. $OPTARG || exit 1
			;;
		esac
		;;
	l)	case $OPTARG in
		*\?*)	. ${OPTARG%%\?*} || exit 1
			eval "license+=( ${OPTARG#*\?} )"
			;;
		*)	path=$PATH
			[[ $OPTARG == */* ]] && PATH=${OPTARG%/*}:$PATH
			. $OPTARG || exit 1
			PATH=$path
			;;
		esac
		;;
	o)	options "$OPTARG"
		;;
	t)	: ignored :
		;;
	x)	: ignored :
		;;
	*)	usage
		;;
	esac
done
shift OPTIND-1
case $# in
1)	file=$1
	if	[[ ! -f $file ]]
	then	print -u2 $command: $file: cannot read
		exit 1
	fi
	x="-m $file"
	;;
*)	x=
	;;
esac

if	[[ ! ${html.http} && ${html.host} ]]
then	html.http=${html.host}/
fi
if	[[ ${html.http} && ${html.http} != *:://* ]]
then	html.http=http://${html.http}
fi
html.http=${html.http%/}
if	[[ ! ${html.man} ]]
then	html.man=../man
fi
if	[[ ${html.man} && ${html.man} != */ ]]
then	html.man=${html.man}/
fi

ds[Cr]='&#169;'
ds[Dt]=$(date -f "%B %d, %Y" $x)
ds[Rf]="[$reference]"
ds[Rg]='&#174;'
ds[CM]='&#169;'
ds[RM]='&#174;'
size_small=9
size_normal=12
size_big=18
ds[SM]="<FONT size=$size_small>[b]SM[/b]</FONT>/g"
ds[TM]="<FONT size=$size_small>[b]TM[/b]</FONT>/g"
tt=code
dd=

map[.Cs]=.EX
map[.Ce]=.EE
map[.Sh]=.SH
map[.Ss]=.SS
map[.Tp]=.TP

H=H$(( head + 1 ))

function warning
{
	print -u2 "$command: warning: ${file:+"$file: "}line $line:" "$@"
}

function getfiles
{
	sed	\
	-e 's%\\".*%%' \
	-e 's%\\(>=%>=%g' \
	-e 's%\\(<=%<=%g' \
	-e 's%\\'\''%'\''%g' \
	-e 's%\\`%`%g' \
	-e 's%\\+%+%g' \
	-e 's%\\0% %g' \
	-e 's%\\|% %g' \
	-e 's%\\\^% %g' \
	-e 's%\\ % %g' \
	-e 's%\\(+-%\&#177;%g' \
	-e 's%\\(-%=%g' \
	-e 's%\\(ap%~%g' \
	-e 's%\\(bu%\&#183;%g' \
	-e 's%\\(bv%|%g' \
	-e 's%\\(co%\&#169;%g' \
	-e 's%\\(dg%\&#167;%g' \
	-e 's%\\(fm%'\''%g' \
	-e 's%\\(rg%\&#174;%g' \
	-e 's%\\(sq%\&#164;%g' \
	-e 's%\\(\*\([*`'\'']\)%\1%g' \
	-e 's%\\\*\([*`'\'']\)%\1%g' \
	-e 's%\\d\([^\\]*\)\\u%\1%g' \
	-e 's%\\u\([^\\]*\)\\d%\1%g' \
	-e 's%\\v\(.\)-\([^\\]*\)\1\(.*\)\\v\1+*\2\1%\3%g' \
	-e 's%\\v\(.\)+*\([^\\]*\)\1\(.*\)\\v\1-\2\1%\3%g' \
	-e 's%\\h'\''0\*\\w"\([[:lower:]]*:[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\2]]%g' \
	-e 's%\\h'\''0\*\\w"\(/[^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\2]]%g' \
	-e 's%\\h'\''0\*\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\2]]%g' \
	-e 's%\\h'\''0\*1'\''\([^:/'\'']*\)\\h'\''0'\''%[[\1][\1]]%g' \
	-e 's%\\h'\''0\*1'\''\([[:lower:]]*:[^'\'']*\)\\h'\''0'\''%[[\1][\1]]%g' \
	-e 's%\\h'\''0\*1'\''\(/[^'\'']*\)\\h'\''0'\''%[[\1][\1]]%g' \
	-e 's%\\h'\''0\*1'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\1]]%g' \
	-e 's%\\h'\''0/\\w"\([^"]*\)"'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\2]]%g' \
	-e 's%\\h'\''0/1'\''\([^'\'']*\)\\h'\''0'\''%[[\1][\1]]%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s-\1%<FONT size='$size_big'>\2</FONT>/g%g' \
	-e 's%\\s+\(.\)\([^\\]*\)\\s0%<FONT size='$size_big'>\2</FONT>/g%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s+\1%<FONT size='$size_small'>\2</FONT>/g%g' \
	-e 's%\\s-\(.\)\([^\\]*\)\\s0%<FONT size='$size_small'>\2</FONT>/g%g' \
	-e 's%\\f(\(..\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%\\f[PR]%\\fZ%g' \
	-e 's%\\f\(.\)\([^\\]*\)%<\1>\2</\1>%g' \
	-e 's%&lt;\([[:alnum:]][-._[:alnum:]]*@[[:alnum:]_.]*\)&gt;%<FONT size='$size_small'>\&lt;[url=mailto:\1]\1[/url]\&gt;</FONT>/g%g' \
	-e 's%\[[[:upper:]][[:alpha:]]*[[:digit:]][[:digit:]][[:lower:]]*]%*&*%g' \
	-e 's%</*Z>%%g' \
	-e 's%<[146789]>%%g' \
	-e 's%</[146789]>%%g' \
	-e 's%<2>%_%g' \
	-e 's%</2>%_ %g' \
	-e 's%<3>%*%g' \
	-e 's%</3>%* %g' \
	-e 's%<5>%=%g' \
	-e 's%</5>%= %g' \
	-e 's%<B>%*%g' \
	-e 's%</B>%* %g' \
	-e 's%<I>%_%g' \
	-e 's%</I>%_ %g' \
	-e 's%<L>%=%g' \
	-e 's%</L>%= %g' \
	-e 's%<X>%=%g' \
	-e 's%</X>%= %g' \
	-e 's%<CW>%=%g' \
	-e 's%</CW>%= %g' \
	-e 's%<i>\([^<]*\)</i>(\([[:digit:]]\))%[[../man\2/\1.html][\1]]\2%g' \
	-e 's%<b>\([^<]*\)</b>(\([[:digit:]]\))%[[../man\2/\1.html][\1]]\2%g' \
	-e 's%\\s+\(.\)\(.*\)\\s-\1%<FONT size='$size_big'>\2</FONT>/g%g' \
	-e 's%\\s-\(.\)\(.*\)\\s+\1%<FONT size='$size_small'>\2</FONT>/g%g' \
	-e 's%\\e%\&#0092;%g' \
	-e '/^'\''[[:lower:]][[:lower:]]\>/s%.%.%' \
	-e '/^\..*".*\\/s%\\[^\*][^(]%\\&%g' \
	"$@"
}

fill=
spaced=1

function space
{
	flush
	if	(( !spaced ))
	then	spaced=1
		print
	fi
}

function flush
{
	if	[[ $fill ]]
	then	print -r -- "${fill#?}"
		fill=
		spaced=0
	fi
}

function puttext
{
	if	((nf))
	then	print -r -- "${li[lists]}$*"
		spaced=0
	elif	[[ $1 ]]
	then	if	(( spaced ))
		then	spaced=0
			[[ $fill ]] || fill=${li[lists]}
		fi
		[[ $fill == *[[:space:]] ]] || fill+=" "
		fill+=$*
	else	flush
		space
	fi
}

function putop
{
	flush
	if	(( $# ))
	then	print -r -- "${li[lists]}$*"
		spaced=0
	fi
}

function putopn
{
	flush
	if	(( $# ))
	then	fill=" ${li[lists]}$*"
		spaced=1
	fi
}

function putopt
{
	if	[[ $1 ]]
	then	fill+=$*
	fi
}

function getline
{
	integer i n
	typeset data a c d q v x d
	if	(( peek ))
	then	(( peek = 0 ))
		trap 'set -- "${text[@]}"' 0
		return
	fi
	while	:
	do	data=
		while	:
		do	IFS= read -r -u$fd a || {
				if	(( so > 0 ))
				then	eval exec $fd'>&-'
					(( so-- ))
					fd=${so_fd[so]}
					file=${so_file[so]}
					line=${so_line[so]}
					continue
				fi
				return 1
			}
			(( line++ ))
			case $a in
			*\\)	x=${a%%+(\\)}
				a=${a#"$x"}
				if	(( ! ( ${#a} & 1 ) ))
				then	data=$data$x$a
					break
				fi
				data=$data$x${a%?}
				;;
			*\\\})	data=$data${a%??}
				break
				;;
			*)	data=$data$a
				break
				;;
			esac
		done
		case $data in
		*\\[n\*]*)
			a= i=0
			while	:
			do	c=${data:i++:1}
				case $c in
				"")	break
					;;
				\\)	x=${data:i:1}
					case $x in
					[n\*])	(( i++ ))
						c=${data:i++:1}
						case $c in
						\()	c=${data:i:2}
							(( i += 2 ))
							;;
						\[)	c=${data:i}
							c=${c%%]*}
							(( i += ${#c} + 1 ))
							x='*'
							;;
						esac
						case $x in
						n)	a=$a${nr[$c]} ;;
						*)	a=$a${ds[$c]} ;;
						esac
						continue
						;;
					\\)	a=$a$c
						while	:
						do	c=${data:i++:1}
							case $c in
							\\)	;;
							*)	break ;;
							esac
							a=$a$c
						done
						;;
					esac
					;;
				esac
				a=$a$c
			done
			data=$a
			;;
		esac
		case $data in
		.?*)	case $data in
			*[\"]*)
				unset v
				a= i=0 n=0 q=
				while	:
				do	c=${data:i++:1}
					case $c in
					"")	break ;;
					esac
					case $c in
					$q)	q=
						case $c in
						\>)	;;
						*)	continue
							;;
						esac
						;;
					[\"\<])	case $q in
						"")	case $c in
							\<)	q=\>
								;;
							*)	q=$c
								continue
								;;
							esac
							;;
						esac
						;;
					\ |\	)
						case $q in
						"")	case $a in
							?*)	v[n++]=$a
								a=
								;;
							esac
							continue
							;;
						esac
						;;
					esac
					a=$a$c
				done
				case $a in
				?*)	v[n++]=$a ;;
				esac
				set -A text -- "${v[@]}"
				;;
			*)	set -A text -- $data
				;;
			esac
			case ${text[0]} in
			.el|.ie|.if)
				set -- "${text[@]}"
				shift
				x=$1
				shift
				case ${text[0]} in
				.e*)	if	(( nest <= 0 ))
					then	warning "unmatched ${text[0]}"
						n=0
					else	n=$(( ! ${cond[nest--]} ))
					fi
					;;
				.i*)	case $x in
					!*)	x=${x#?}
						n=1
						;;
					*)	n=0
						;;
					esac
					case $x in
					t|\'@(*)\'\1\'|+([\-+[:digit:]])=\1)
						(( n = ! n ))
						;;
					+([\-+[:digit:]])=+([\-+[:digit:]]))
						;;
					[[:digit:]]*[[:digit:]])
						(( n = x ))
						;;
					esac
					case ${text[0]} in
					.ie)	cond[++nest]=$n ;;
					esac
					;;
				esac
				if	(( ! n ))
				then	case $@ in
					\\\{*)	while	read -r -u$fd data
						do	(( line++ ))
							case $data in
							*\\\})	break ;;
							esac
						done
						;;
					esac
					continue
				fi
				set -A text -- "$@"
				case ${text[0]} in
				\\\{*)	text[0]=${text[0]#??} ;;
				esac
				;;
			.so)	x=${text[1]}
				for d in "${dirs[@]}"
				do	if	[[ -f "$d$x" ]]
					then	so_fd[so]=$fd
						(( fd = so + soff ))
						tmp=/tmp/m2h$$
						getfiles "$d$x" > $tmp
						eval exec $fd'< $tmp'
						rm $tmp
						so_file[so]=$file
						file=$d$x
						so_line[so]=$line
						(( line = 0 ))
						(( so++ ))
						continue 2
					fi
				done
				warning "$x: $op cannot read"
				continue
				;;
			.xx)	data=
				set -- "${text[@]}"
				shift
				while	:
				do	case $# in
					0)	break ;;
					esac
					nam=${1%%=*}
					case $nam in
					no?*)	nam=${nam#no} val=0 ;;
					*)	val=${1#*=} ;;
					esac
					shift
					case $nam in
					label|link*|ref)
						case $val in
						*'	'*)
							url=${val%%'	'*}
							txt=${val#*'	'}
							;;
						*'\\t'*)
							url=${val%%'\\t'*}
							txt=${val#*'\\t'}
							;;
						*)	url=$val
							txt=$val
							;;
						esac
						case $url in
						*[:/.]*)	pfx= ;;
						*)		pfx='#' ;;
						esac
						case $url in
						*'${html.'*'}'*)
							eval url=\"$url\"
							;;
						esac
						case $nam in
						label)	if	(( labels >= 0 ))
							then	nam=name
								label[labels++]=$txt
								puttext "[[$url][$txt]]"
							fi
							;;
						link*)	tar=
							case $nam in
							link)	case $frame$top$vg in
								?*)	case $url in
									*([[:lower:]]):*|/*)
										tar=" target=_top"
										;;
									esac
									;;
								esac
								;;
							esac
							nam=href
							if	[[ ! $pfx && $url != *://* ]]
							then	url=${html.http}$url
							fi
							data="${data}[[$pfx$url][$txt]]"
							;;
						esac
						;;
					esac
				done
				case $data in
				'')	continue ;;
				esac
				set -A text -- "$data"
				break
				;;
			esac
			case ${text[0]} in
			@($primary))
				: primary macros take precedence
				;;
			.[BILMRUX]?([BILMRUX])|.F|.FR|.MW|.RF)
				case $macros:${text[0]} in
				mm:.RF)	break ;;
				esac
				typeset font1 font2 op
				set -- "${text[@]}"
				op=$1
				shift
				case $op in
				.[BIL]R)case $#:$2 in
					2':('[[:digit:]]')'*([,.?!:;]))
						x=${2#'('*')'}
						y=${2%$x}
						n=$y
						case $op in
						.B*)	font1=b ;;
						.L*)	font1=$tt ;;
						*)	font1=i ;;
						esac
						case $macros in
						man)	set -A text -- "[[../man$n/$1.html][$1]]$y$x" ;;
						*)	set -A text -- "[[${html.http}${html.man}man$n/$1.html][$1]]$y$x" ;;
						esac
						break
						;;
					esac
					;;
				.F)	op=.L ;;
				.FR)	op=.LR ;;
				.MW)	op=.L ;;
				.RF)	op=.RL ;;
				esac
				case $#:$macros:$op in
				0:*)	;;
				*:man:.?)
					set -- "$*"
					;;
				esac
				case $# in
				0)	getline
					set -- "$*"
					;;
				esac
				set -- "$indent""$@"
				case $font in
				"")	data=
					;;
				?*)	data="$font"
					font=
					;;
				esac
				font1=${op#.}
				case $font1 in
				?)	font2=R
					;;
				*)	font2=${font1#?}
					font1=${font1%?}
					;;
				esac
				case $font1 in
				B)	font1='*' ;;
				I)	font1='__' ;;
				[LMX])	font1='=' ;;
				R)	font1='' ;;
				esac
				case $font2 in
				B)	font2='*' ;;
				I)	font2='__' ;;
				[LMX])	font2='=' ;;
				R)	font2='' ;;
				esac
				font=$font2
				while	:
				do	case $# in
					0)	break ;;
					esac
					case $font in
					$font2)	font=$font1 ;;
					*)	font=$font2 ;;
					esac
					case $1 in
					"")	;;
					*"<FONT size="*)
						case $font in
						"")	data="$data$1" ;;
						*)	data="$data$font$1$font " ;;
						esac
						;;
					*)	case "$1 $2" in
						*"<FONT size="*)
							case $font in
							"")	data="$data$1 $2" ;;
							*)	data="$data$font$1 $2$font " ;;
							esac
							shift
							;;
						*)	case $font in
							"")	data="$data$1" ;;
							*)	data="$data$font$1$font" ;;
							esac
							;;
						esac
						;;
					esac
					shift
				done
				font=
				set -A text -- $data
				;;
			.PD)	case $1 in
				0)	pd= ;;
				*)	pd=$nl ;;
				esac
				continue
				;;
			.PF|.PH)continue
				;;
			.SB)	set -- "${text[@]}"
				shift
				case $# in
				0)	getline ;;
				esac
				set -A text -- "<FONT size=$size_small>*""$@""*</FONT>/g"
				;;
			.SG)	continue
				;;
			.SM)	set -- "${text[@]}"
				shift
				case $# in
				0)	getline ;;
				esac
				set -A text -- "<FONT size=$size_small>""$@""</FONT>/g"
				;;
			*)	x=${map[${text[0]}]}
				case $x in
				?*)	text[0]=$x ;;
				esac
				;;
			esac
			;;
		*)	set -A text -- "$data"
			;;
		esac
		break
	done
	trap 'set -- "${text[@]}"' 0
}

function heading
{
	typeset op=$1 i o options beg
	integer count

	shift
	case $op in
	.H)	case $# in
		0)	count=1 ;;
		*)	count=$1; shift ;;
		esac
		options=
		;;
	.H*|.AS)count=1
		;;
	*)	count=2
		;;
	esac
	case $* in
	"")	putop
		;;
	*)	space
		beg=---+
		while (( count-- ))
		do	beg=$beg+
		done
		putop "$beg$*"
		space
		;;
	esac
}

function tbl_attributes
{
	typeset d f i u x
	typeset -F0 w
	case $1 in
	[aAcC]*)a="$a align=center" ;;
	[lL]*)	a="$a align=left" ;;
	[nN]*)	a="$a align=right char=." ;;
	[rR]*)	a="$a align=right" ;;
	esac
	case $i in
	*[wW]\(+([0-9.])*\)*)
		x=${i##*[wW]\(}
		x=${x%%\)*}
		u=${x##+([0-9.])}
		x=${x%$u}
		case $u in
		c)	(( w=x*75/2.54 )) ;;
		i)	(( w=x*75 )) ;;
		m)	(( w=x*75*12/72 )) ;;
		n)	(( w=x*75*12/72/2 )) ;;
		p)	(( w=x*75/72 )) ;;
		P)	(( w=x*75/6 )) ;;
		*)	(( w=x*75*12/72/2 )) ;;
		esac
		a="$a width=$w"
		;;
	esac
	case $i in
	*[bB]*)	b="$b*" e="*$e"
		;;
	esac
	case X$i in
	*[!0-9.][iI]*)
		b="$b_" e="_$e"
		;;
	esac
}

dirs[++ndirs]=""
for i
do	if [[ -d $i ]]
	then	dirs[++ndirs]=$i/
	else	files[++nfiles]=$i
		if [[ $i == */* ]]
		then	dirs[++ndirs]=${i%/*}/
		fi
	fi
done
document="${files[@]}"

getfiles "${files[@]}" |
while	:
do	getline || {
		[[ $title != '' ]] && break
		set -- .TL
	}
	case $1 in
	.)	: comment
		;;
	.*)	op=$1
		shift
		case $op in
		.AC)	: ignore $op
			;;
		.AE)	: ignore $op
			;;
		.AF)	case $mm_AF_cur in
			"")	mm_AF_cur="[i]$*" ;;
			*)	mm_AF_cur="${mm_AF_cur}${nl}$*" ;;
			esac
			;;
		.AL|.[IR]S|.VL)
			case $macros:$op in
			mm:.RS)	
				Rf="[$reference]"
				references="$references$nl   \$ [$reference]:"
				while	getline
				do	case $1 in
					.RF)	break ;;
					esac
					references="$references$nl   $*"
				done
				(( reference++ ))
				continue
				;;
			esac
			type[++lists]=.AL
			list[lists]=DL
			li[lists]=${li[lists-1]}$LI
			lc[lists]='$'
			case $op in
			.AL)	case $1 in
				'')	type[++lists]=.al
					list[lists]=OL
					li[lists]=${li[lists-1]}
					lc[lists]='-'
					;;
				[[:alnum:]])
					type[++lists]=.al
					list[lists]=OL
					li[lists]=${li[lists-1]}
					case $1 in
					[[:digit:]])
						lc[lists]=1.
						;;
					[iI])	lc[lists]=$1.
						;;
					[[:lower:]])
						lc[lists]=a.
						;;
					[[:upper:]])
						lc[lists]=A.
						;;
					esac
					;;
				esac
				;;
			.[IR]S)	;;
			.VL)	case $1 in
				?*)	type[++lists]=.al
					list[lists]=DL
					li[lists]=${li[lists-1]}
					lc[lists]='$'
					;;
				esac
				;;
			esac
			;;
		.AS|.H|.HU|.SH|.SS|.ce|.CE)
			if ((nf))
			then	nf=0
				putop "}"
			fi
			if ((ce))
			then	ce=0
			fi
			case $hp in
			?*)	indent=${indent#$hp}
				hp=
				;;
			esac
			if	(( lists > pp ))
			then	case ${type[@]:0:lists} in
				*.[Aa][Ll]*|*.[IiTt][Pp]*)
					while	:
					do	case ${type[lists]} in
						.[Aa][Ll]|.[IiTt][Pp])
							case ${type[lists]} in
							.AL|.IP|.TP)break ;;
							esac
							;;
						*)	break
							;;
						esac
					done
					;;
				esac
			fi
			(( pp = lists ))
			end=
			case ${mm.title} in
			?*)	beg="${beg}<FONT size=$size_big color=blue>"
				end="</FONT>/g$end"
				space
				putop "$beg ${mm.title} $end"
				space
				mm.title=
				case ${mm.author} in
				?*)	IFS=:
					set -- ${mm.author#:} : "$@"
					IFS=$ifs
					while	:
					do	case $# in
						0)	break ;;
						esac
						x=$1
						shift
						case $x in
						:)	break ;;
						esac
						putop "$x"
					done
					;;
				esac
				case $mm_AU in
				?*)	putop "${mm_AU#?}"
					case $mm_AF_cur in
					?*)	mm_AF="${mm_AF_cur}[/i]"
						;;
					esac
					case $mm_AF in
					?*)	putop "$mm_AF" ;;
					esac
					;;
				esac
				;;
			esac
			case $op in
			.AS)	heading $op Abstract
				;;
			.ce)	case $# in
				0)	count=1 ;;
				*)	count=$1 ;;
				esac
				while	(( count-- > 0 )) && read -r data
				do	putop "$data"
				done
				;;
			.CE)	;;
			.S[HS])	macros=man
				while	(( lists > 0 ))
				do	case ${type[lists--]} in
					.S[HS])	break ;;
					esac
				done
				type[++lists]=$op
				list[lists]=DL
				li[lists]=${li[lists-1]}$LI
				case $op in
				.SS)	type[++lists]=.XX
					list[lists]=DL
					li[lists]=${li[lists-1]}
					;;
				esac
				case $op in
				.SH)	heading .H 2 "$@" ;;
				*)	heading .H 3 "$@" ;;
				esac
				type[++lists]=.XX
				list[lists]=DL
				li[lists]=${li[lists-1]}
				lc[lists]=
				;;
			*)	heading $op "$@"
				;;
			esac
			;;
		.AT)	: ignore $op
			;;
		.AU)	case $mm_AF_cur in
			?*)	case $mm_AF in
				?*)	case $mm_AU in
					?*)	mm_AU="${mm_AU}${mm_AF}" ;;
					esac
					;;
				esac
				mm_AF="${mm_AF_cur}[/i]"
				mm_AF_cur=""
				;;
			esac
			mm_AU="${mm_AU}$nl$1"
			;;
		.BL)	i=
			for ((n = 1; n <= lists; n++))
			do	i=$i${list[n]}
			done
			case $i in
			*UL*UL*)i=disc ;;
			*UL*)	i=circle ;;
			*)	i=square ;;
			esac
			type[++lists]=.AL
			list[lists]=UL
			li[lists]=${li[lists-1]}$LI
			lc[lists]='*'
			;;
		.BP)	unset parm
			while	[[ $1 == *=* ]]
			do	eval parm="( ${1%%=*}='${1#*=}' )"
				shift
			done
			unset oparm
			oparm=$parm
			i=$1
			if	[[ $i == *.@(gif|png) ]]
			then	for i
				do	f=
					for d in "${dirs[@]}"
					do	if [[ -f "$d$i" ]]
						then	f=$d$i
							break
						fi
					done
					if [[ ! $f ]]
					then	print -u2 "$command: warning: $i: data file not found"
					fi
					if	[[ ! ${oparm.alt} ]]
					then	u=${i##*/}
						u=${i%.*}
						parm=( alt=$u )
					fi
					if	[[ ! ${oparm.title} ]]
					then	u=${i##*/}
						u=${i%.*}
						if	[[ ${parm.category} ]]
						then	u="${parm.category} $u"
						elif	[[ ${oparm.category} ]]
						then	u="${oparm.category} $u"
						fi
						parm=( title=$u )
					fi
					putop "[img=\"$i\"]"
				done
			else	i=${i%.*}.gif
				case $frame in
				?*)	[[ -f $frame-$i ]] && i=$frame-$i ;;
				esac
				f=
				for d in "${dirs[@]}"
				do	if	[[ -f "$d$1" ]]
					then	f=$d$1
						break
					fi
				done
				if [[ ! $f ]]
				then	print -u2 "$command: $1: data file not found"
				elif [[ $f -nt $i ]]
				then	ps2gif $f $i
				fi
				putop "[img=\"$i\"]"
			fi
			;;
		.CT)	: ignore $op
			;;
		.DE|.fi)
			if ((nf))
			then	nf=0
				putop "</verbatim>"
			fi
			;;
		.DF|.DS|.nf)
			if ((!nf))
			then	nf=1
				putop "<verbatim>"
			fi
			;;
		.DT)	case $macros in
			man)	;;
			*)	putop "${ds[Dt]}" ;;
			esac
			;;
		.EE)if ((nf))
			then	nf=0
				putop "</verbatim>"
			fi
			if [[ $fg ]]
			then	putop "<FONT color=blue>${fg}</FONT>"
			fi
			indent=${indent#$inch}
			;;
		.EX)if ((!nf))
			then	nf=1
				putop "<verbatim>"
			fi
			indent=$inch$indent
			case $# in
			2)	fg="Figure $1: $2" ;;
			*)	fg= ;;
			esac
			;;
		.FE)	putop " ] "
			;;
		.FG)	putop "<FONT color=blue>Figure $figure: $*</FONT>"
			(( figure++ ))
			;;
		.FS)	putop " [ "
			;;
		.HP|.LP|.P|.PP)
			case $hp in
			?*)	indent=${indent#$hp}
				hp=
				;;
			esac
			if	(( lists > pp ))
			then	case ${type[@]:0:lists} in
				*.[Aa][Ll]*|*.[IiTt][Pp]*)
					while	:
					do	case ${type[lists]} in
						.[Aa][Ll]|.[IiTt][Pp])
							case ${type[lists]} in
							.AL|.IP|.TP)break ;;
							esac
							;;
						*)	break
							;;
						esac
					done
					;;
				esac
			fi
			(( pp = lists ))
			space
			;;
		.HY)	: ignore $op
			;;
		.IP|.LI|.TF|.TP)
			case $macros:$op in
			mm:.TP)	continue ;;
			esac
			case $op in
			.IP|.LP|.TF|.TP)OP=$op
				case ${type[lists]} in
				$op|$OP);;
				*)	type[++lists]=$op
					list[lists]=DL
					li[lists]=${li[lists-1]}$LI
					lc[lists]=
					putop
					;;
				esac
				case $op in
				.IP|.LP|.TF)
					set -- "=$*="
					;;
				.TP)	getline
					;;
				esac
				;;
			esac
			if	[[ ${list[lists]} ]]
			then	if	[[ ${list[lists]} == DL ]] && (( ! $# ))
				then	getline
				fi
				if	[[ ${lc[lists]} == '$' ]]
				then	bold=1 colon=1
				else	bold=0 colon=0
				fi
				arg=$*
				if	[[ $arg == _*_ ]]
				then	if	(( bold ))
					then	arg=_${arg}_
						bold=0
					fi
				elif	[[ $arg == _* || $arg == *_ || $arg == *'*'*'*'* ]]
				then	bold=0
				fi
				if	(( bold ))
				then	arg='*'${arg}'*'
				fi
				if	(( colon ))
				then	arg=${arg}:
				fi
				putopn "${lc[lists]} $arg"
			else	warning "$op: no current list"
			fi
			;;
		.IX)	: ignore $op
			;;
		.LE|.[IR]E)
			case ${type[@]} in
			*.[Aa][Ll]*)
				flush
				while	(( lists > 0 ))
				do	case ${type[lists--]} in
					.AL)	break ;;
					esac
				done
				;;
			*)	warning "$op: no current list type"
				;;
			esac
			case $op:$pd in
			.[IR]E:?*)	putop ;;
			esac
			;;
		.LX)	: ignore $op
			;;
		.MT)	macros=mm
			;;
		.ND|.Dt)ds[Dt]=$*
			;;
		.NL)	type[++lists]=.AL
			list[lists]=OL
			li[lists]=${li[lists-1]}$LI
			lc[lists]=1.
			;;
		.OK)	mm.keywords="$*"
			;;
		.OP)	: .OP opt arg arg-append arg-prepend
			x="$4[b]&#45;$1[/b][i]$2[/i]"
			case $3 in
			'[]')	x="[ $x ]" ;;
			?*)	x="$x$3" ;;
			esac
			putop "$x"
			;;
		.PM)	case ${html.company} in
			'')	pm= ;;
			*)	pm="${html.company//\&/\&amp\;} " ;;
			esac
			case $1 in
			'')	pm= ;;
			C|NDA)	pm="${pm}CONFIDENTIAL" ;;
			RG)	pm="${pm}PROPRIETARY (REGISTERED)" ;;
			RS)	pm="${pm}PROPRIETARY (RESTRICTED)" ;;
			*)	pm="${pm}PROPRIETARY" ;;
			esac
			case $pm in
			?*)	pm="<FONT color=blue>${pm}</FONT>"
			esac
			;;
		.PU)	: ignore $op
			;;
		.SA)	: ignore $op
			;;
		.SU)	: ignore $op
			;;
		.TH|.TL): .TL junk junk
			: .TH item section foot_center foot_left head_center
			title=1
			case $macros:$op in
			:.TH)	macros=man ;;
			:.TL)	macros=mm ;;
			esac
			case $op in
			.TH)	case $3 in
				?*)	dc[++dcs]=$3 ;;
				esac
				case $4 in
				?*)	dl[++dls]=$4 ;;
				esac
				case $5 in
				'')	sec=$(set --'???'MAN=$2 2>&1)
					sec=$ver$msc$cmp$sec
					;;
				*)	sec=$5
					;;
				esac
				mm.title="$sec -- $1($2)"
				;;
			.TL)	getline || break
				case ${html.title} in
				'')	mm.title=$* ;;
				esac
				;;
			esac
			;;
		.TM)	: ignore $op
			;;
		.TX)	: ignore $op
			;;
		.UC)	: ignore $op
			;;
		.VE)	: ignore $op
			;;
		.VS)	: ignore $op
			;;
		.al)	: ignore $op
			;;
		.bd)	: ignore $op
			;;
		.br)	putop
			;;
		.de|.am.ig)
			end=..
			case $#:$op in
			0:*)	;;
			*:.ig)	end=$1 ;;
			esac
			: ignore $op to $end
			while	getline
			do	case $1 in
				$end)	break ;;
				esac
			done
			;;
		.ds)	op=$1
			shift
			ds[$op]=$*
			;;
		.f[tBILPR])
			case $op in
			.f[BILPR])
				set -- ${op#.f}
				;;
			esac
			case $1 in
			5|TT)	font=$tt
				;;
			B)	font=b$dd
				;;
			I)	font=i$dd
				;;
			*)	case $font in
				?*)	puttext "$font"
					font=
					;;
				esac
				;;
			esac
			if	[[ $font ]]
			then	puttext "$font"
			fi
			;;
		.fp)	: ignore $op
			;;
		.hc)	: ignore $op
			;;
		.hw)	: ignore $op
			;;
		.hy)	: ignore $op
			;;
		.in)	indent_prev=$indent
			case $1 in
			"")	i=$indent_prev; indent_prev=$indent; indent=$i ;;
			-*)	indent=${indent#$inch} ;;
			+*)	indent=$inch$indent ;;
			*)	indent=$inch ;;
			esac
			;;
		.lf)	: ignore $op
			;;
		.ll)	: ignore $op
			;;
		.ne)	: ignore $op
			;;
		.nh)	: ignore $op
			;;
		.nr)	op=$1
			shift
			nr[$op]=$*
			;;
		.ns)	: ignore $op
			;;
		.ps)	case $1 in
			-*)	putop "<FONT size=$size_small>"
				;;
			+*)	putop "<FONT size=$size_big>"
				;;
			esac
			;;
		.sh)	case $HTMLPATH in
			'')	;;
			*)	so_fd[so]=$fd
				(( fd = so + soff ))
				file=/tmp/m2h$$
				path=$PATH
				eval PATH=$HTMLPATH "$*" > $file
				PATH=$path
				eval exec $fd'< $file'
				rm $file
				so_file[so]=$file
				file=$1
				so_line[so]=$line
				(( line = 0 ))
				(( so++ ))
				;;
			esac
			;;
		.sn)	for d in "${dirs[@]}"
			do	if	[[ -f "$d$1" ]]
				then	cat "$d$1"
					continue 2
				fi
			done
			warning "$1: $op cannot read"
			;;
		.sp|.SP)space
			;;
		.ta)	: ignore $op
			;;
		.ti)	: ignore $op
			;;
		.ul)	: ignore $op
			;;
		*)	warning "$op: unknown op"
			;;
		esac
		;;
	""|[\ \	]*)
		case $macros in
		man)	leading=1
			prefix="<verbatim>"
			blank=
			while	:
			do	case $1 in
				*([\ \	]))
					case $leading in
					1)	leading= ;;
					*)	blank=$'\n' ;;
					esac
					;;
				[\ \	]*)
					puttext "$prefix$blank$inch$indent$*"
					blank=
					leading=
					prefix=
					;;
				*)	(( peek = 1 ))
					break
					;;
				esac
				getline || break
			done
			case $prefix in
			"")	putop "</verbatim>"
				;;
			esac
			;;
		*)	puttext "$indent$*"
			;;
		esac
		;;
	*)	puttext "$indent$*"
		;;
	esac
done
flush
case $references in
?*)	heading .H 1 References
	puttext "$references"
	flush
	;;
esac
exit $code
