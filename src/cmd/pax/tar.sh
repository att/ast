########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1987-2012 AT&T Intellectual Property          #
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
:
# @(#)tar.sh (AT&T Labs Research) 1989-04-06
#
# tar -> pax interface script
#

command=tar

#Usage: $command c[bBfkLopvVwzZ[[0-9][hlm]]] [tarfile] [exclude-file]
#       $command r[bBfkLvVwzZ[[0-9][hlm]]] [files ...]
#       $command t[bBfkvX[[0-9][hlm]] [tarfile] [exclude-file]
#       $command u[bBfkLvVwzZ[[0-9][hlm]]] [tarfile]
#       $command x[bBfkmopvVw[[0-9][hlm]]] [tarfile] [exclude-file]"

usage=$'
[-?\n@(#)$Id: tar (AT&T Labs Research) 2012-05-07 $\n]
'$USAGE_LICENSE$'
[+NAME?tar - create tape archives, and add or extract files]
[+DESCRIPTION?The \btar\b utility is a compatibility script on top of
    the \bpax\b(1) utility. It archives and extracts files to and from a
    single file called a \atarfile\a. A \atarfile\a can be any file,
    including a magnetic tape. \btar\b\'s actions are controlled by the
    \akey\a. The \akey\a is an \aoptions\a string containing one of the
    single-character function letters \(\bc\b, \br\b, \bt\b, \bu\b, or
    \bx\b\) and other function modifiers as needed, depending on the
    function character used. The leading - is optional for the
    single-character function letters. Some of the function modifiers
    (\bb\b, \bf\b, \bX\b) take operands. Operands of these function
    modifiers appear after the \aoptions\a string instead of immediately
    following the corresponding function modfier character in the
    \aoptions\a string. Other command-line arguments are filenames \(or
    directory names\) that specify which files are to be archived or
    extracted. In all cases, appearance of a directory name refers to the
    files and \(recursively\) subdirectories of that directory.]
[A:catenate|concatenate?Append \btar\b files to an archive.]
[c:create?Create a new archive.]
[r:append?Append files to the end of an archive.]
[t:list?List the contents of an archive.]
[x:extract|get?Extract files from an archive.]
[13:atime-preserve?Do not change access times on dumped files.]
[b:block-size]:[N?block size of Nx512 bytes (default N=20).]
[B:read-full-blocks?Reblock as we read (for reading 4.2BSD pipes).]
[E?Write a tar file with extended headers.]
[f:file]:[file?Use archive \afile\a or device (default /dev/rmt0).]
[h:logical|dereference?Follow symbolic links.]
[j:bzip?Filter the archive through \bbzip2\b(1).]
[J:xz?Filter the archive through \bxz\b(1).]
[k?On archive read only extract files that are newer than the target
    files.]
[l?Link. Complain if tar cannot resolve all of the links to files being
    archived.]
[L:tape-length]:[N?Change tapes after writing N*1024 bytes.]
[m:modification-time?Do not extract file modified time.]
[o?Assign to extracted files the user and group identifiers of the user
    running the tar program, rather than those on tarfile.]
[p:same-permissions|preserve-permissions?Extract all protection
    information.]
[v:verbose?Verbosely list files processed.]
[V:label]:[name?Create archive with volume name \aname\a.]
[w:interactive|confirmation?Ask for confirmation for every action.]
[23:exclude-file]:[pattern?Do not extract matching the \bksh\b(1)
    \apattern\a from the archive.]
[z:gzip?Filter the archive through \bgzip\b(1).]
[Z:compress?Filter the archive through \bcompress\b(1).]
[33:show?Show but do not execute the underlying \bpax\b(1) command.]
[0|1|2|3|4|5|6|7|8|9?Alternative drive number.]

[operands] files ...

[+SEE ALSO?\bar\b(1), \bbzip2\b(1), \bchown\b(1), \bcompress\b(1),
    \bcpio\b(1), \bcsh\b(1), \bgzip\b(1), \bksh\b(1), \bls\b(1),
    \bmt\b(1), \bpax\b(1), \bumask\b(2), \bxz\b(1), \benviron\b(5)]
'

typeset -a options
integer noptions=0

options[noptions++]="-P"
eval=eval
file=""
list=false
r_ok=true
w_ok=true
arg=""
lastopt=""
mode=""
format="ustar"
zip=""
optsWargs=""
excludedFiles=""
directedToExclude=false
fileSpecified=false
tapeSpecified=false

usage()
{
	OPTIND=0
	getopts "$usage" opt
	exit 2
}

process_option()
{
	case $1 in
	[0-9])	case $fileSpecified in
		true)	print -u2 "$command: cannot specify both file and tape"; exit 1 ;;
		*)	file="--tape=$1"
			tapeSpecified=true
			;;
		esac
		;;
	13)	options[noptions++]="--atime"	;;
	23)	if [[ $# == 0 ]]
		then
			print -u2 "$command: excluded file name argument expected"; exit 1
		else
			excludedFiles+=" $OPTARG"
			directedToExclude=true
			w_ok=false
		fi
		;;
	33)	set --showme
		eval=
		;;
	[hlm])	case $lastopt in
		[0-9])	file="${file}$1"	;;
		*)	case $1 in
			h)	options[noptions++]="-L"	;;
			l)	;;
			m)	options[noptions++]="-pam"	;;
			esac
			;;
		esac
		;;
	b)	if [[ $2 == "short form" ]]
		then
			optsWargs+="b "
		elif [[ $# == 0 ]]
		then
			print -u2 "$command: blocking factor argument expected"; exit 1
		else
			options[noptions++]="-b"
			options[noptions++]="${OPTARG}b"
		fi
		;;
	c)	mode="-w" ; r_ok=false ;;
	f)	if [[ $tapeSpecified == true ]]
		then
			print -u2 "$command: cannot specify both file and tape"; exit 1
		elif [[ $2 == "short form" ]]
		then
			optsWargs+="f "
			fileSpecified=true
		elif [[ $# == 0 ]]
		then
			print -u2 "$command: file name argument expected"; exit 1
		else
			case $1 in	# Retain most recent file operand
			-)	file="" ;;
			*)	file="--file=$OPTARG" ;;
			esac
			fileSpecified=true
		fi
		;;
	j)	zip=":bzip" ;;
	k)	options[noptions++]="-u"	;;
	o)	options[noptions++]="--owner=${USER:-${LOGNAME:-$(id -un)}}" ;;
	p)	options[noptions++]="-pe" ;;
	r)	mode="-w" ; r_ok=false ; options[noptions++]="-a" ;;
	t)	mode="-r" ; w_ok=false ; list=true ;;
	u)	mode="-w" ; r_ok=false ; options[noptions++]="-u" ;;
	v)	options[noptions++]="-v" ;;
	w)	options[noptions++]="-o"; options[noptions++]="yes" ;;
	x)	mode="-r" ; w_ok=false ;;
	z)	zip=":gzip"	;;
	A)	mode="-w" ; r_ok=false ; options[noptions++]="-a" ;;
	B)	options[noptions++]="-b"; options[noptions++]="10ki"	;;
	E)	format="pax"	;;
	J)	zip=":xz" ;;
	L)	if [[ $2 == "short form" ]]
		then
			optsWargs+="L "
		elif [[ $# == 0 ]]
		then
			print -u2 "$command: maxout argument expected"; exit 1
		else
			options[noptions++]="--maxout=$(($OPTARG*1024))"
		fi
		;;
	V)	if [[ $2 == "short form" ]]
		then
			optsWargs+="V "
		elif [[ $# == 0 ]]
		then
			print -u2 "$command: volume label argument expected"; exit 1
		else
			options[noptions++]="--label=$OPTARG"
		fi
		;;
	Z)	zip=":compress" ;;
	*)	usage ;;
	esac
	lastopt=$1
}

case $1 in
--*)	while getopts "$usage" opt
	do	process_option "$opt"	"long form"
	done
	shift $((OPTIND-1))
	;;
"")	usage
	;;
*)	if [[ ${1:0:1} == '-' ]]
	then	arg=${1:1}
	else	arg=$1
	fi
	shift
	while [[ $arg ]]
	do	process_option ${arg:0:1} "short form" || usage
		arg=${arg#?}
	done
	;;
esac

block=""
maxout=""
label=""
for i in $optsWargs
do
	case $i in
	b)	case $# in
		0)	print -u2 "$command: blocking factor argument expected"; exit 1 ;;
		esac
		block=$1		# Retain most recent block operand
		shift
		;;
	f)	case $# in
		0)	print -u2 "$command: file name argument expected"; exit 1 ;;
		esac
		case $1 in		# Retain most recent file operand
		-)	file=""			;;
		*)	file="--file=$1"	;;
		esac
		shift
		;;
	L)	case $# in
		0)	print -u2 "$command: maxout argument expected"; exit 1 ;;
		esac
		maxout=$(($1*1024))	# Retain most recent maxout operand
		shift
		;;
	V)	case $# in
		0)	print -u2 "$command: volume label argument expected"; exit 1 ;;
		esac
		label=$1		# Retain most recent label operand
		shift
		;;
	esac
done
if [[ $block ]]
then	options[noptions++]="-b"; options[noptions++]="${block}b"	# Take most recent block operand short form
fi
if [[ $maxout ]]
then	options[noptions++]="--maxout=$maxout"	# Take most recent maxout operand short form
fi
if [[ $label ]]
then	options[noptions++]="--label=$label"	# Take most recent label operand short form
fi

if [[ $directedToExclude == true ]]
then
	exclude=""
	prefix="!("			# Build list of files to exclude
	for i in $excludedFiles
	do	exclude+=$prefix$i
		prefix="|"
	done
	if	[[ $exclude ]]
	then	exclude+=")"
		set -- $exclude
	fi
fi

case $mode in
-r)	case $r_ok in
	false)	print -u2 "$command: options inconsistent with archive read"; exit 1 ;;
	esac
	case $list in
	true)	mode="" ;;
	esac
	;;
-w)	case $w_ok in
	false)	print -u2 "$command: options inconsistent with archive write"; exit 1 ;;
	esac
	case $# in
	0)	set "." ;;
	esac
	options[noptions++]="-x"; options[noptions++]="$format$zip"
	;;
*)	print -u2 "$command: one of Acrtx must be specified"; exit 1 ;;
esac

[[ $file ]] && options[noptions++]="$file"

; pax $mode "${options[@]}" "$@" 
