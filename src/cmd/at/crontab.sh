########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1996-2011 AT&T Intellectual Property          #
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
# Glenn Fowler
# AT&T Research

command=crontab
prefix=${0##*/}
case $prefix in
*_*)	prefix=${prefix%%_*}_ ;;
*)	prefix= ;;
esac
cron="${prefix}at -qc"
file=$HOME/.$command
replace=
temp=${TMPDIR:-/tmp}/cron$$
op=

case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	usage=$'
[-?
@(#)$Id: crontab (AT&T Labs Research) 2006-05-17 $
]
'$USAGE_LICENSE$'
[+NAME?crontab - schedule periodic background work]
[+DESCRIPTION?\bcrontab\b creates, replaces or edits the calling user crontab
	entry; a crontab entry is a list of commands and the times at which
	they are to be executed. The new crontab entry can be input by
	specifying \afile\a, or input from standard input if \afile\a is
	omitted, or by using an editor, if \b--edit\b is specified. The
	actual crontab entry is maintained in the file \b$HOME/.crontab\b.]
[+?Upon execution of a command from a crontab entry, the implementation will
	supply a default environment, defining at least the following
	environment variables:]{
	[+HOME?A pathname of the calling user home directory.]
	[+LOGNAME?The calling user login name.]
	[+PATH?A string representing a search path guaranteed to find all
		of the standard utilities.]
	[+SHELL?A pathname of the command interpreter \bsh\b(1).]
}
[+?If standard output and standard error are not redirected by commands
	executed from the crontab entry, any generated output or errors will
	be mailed to the calling user.]
[+?Users are permitted to use \bcrontab\b if their names appear in the file
	\b/usr/lib/cron/cron.allow\b. If that file does not exist, the file
	\b/usr/lib/cron/cron.deny\b is checked to determine if the calling user
	should be denied access to \bcrontab\b. If neither file exists, only
	a process with appropriate privileges is allowed to submit a job. If
	only \bcron.deny\b exists and is empty, global usage is permitted. The
	\bcron.allow\b and \bcron.deny\b files consist of one user name per
	line.]
[+?In this implementation, each command in the \bcrontab\b file represents
	a single \bat\b(1) \ajob\a. Only one job per command will be executed.
	A new job for a given command will not be scheduled until the previous
	job for that command, if any, has exited.]
[e:edit?Edit a copy of the calling user crontab entry, or create an empty
	entry to edit if the crontab entry does not exist. When editing is
	complete, the entry will be installed as the calling user crontab
	entry. The editor used is defined by the \bEDITOR\b environment
	variable. The default editor is \bvi\b(1).]
[l:list?List the calling user crontab entry.]
[r:remove|delete?Remove the calling user crontab entry.]

[ file ]

[+INPUT FILES?A crontab entry must be a text file consisting of lines of six
	fields each. The fields must be separated by blank characters. The
	first five fields must be integer patterns that specify the
	following:]{
	[+1.?Minute (0-59)]
	[+2.?Hour (0-23)]
	[+3.?Day of the month (1-31)]
	[+4.?Month of the year (1-12)]
	[+5.?Day of the week (0-6 with 0=Sunday)]
}
[+?Each of these patterns can be either an asterisk (meaning all valid
	values), an element or a list of elements separated by commas.
	An element must be either a number or two numbers separated by a
	hyphen (meaning an inclusive range). The specification of days can
	be made by two fields (day of the month and day of the week). If
	month, day of month and day of week are all asterisks, every day
	will be matched. If either the month or day of month is specified
	as an element or list, but the day of week is an asterisk, the
	month and day of month fields will specify the days that match. If
	both month and day of month are specified as asterisk, but day of
	week is an element or list, then only the specified days of the
	week will match. Finally, if either the month or day of month is
	specified as an element or list, and the day of week is also
	specified as an element or list, then any day matching either the
	month and day of month or the day of week, will be matched.]
[+?The sixth field of a line in a crontab entry is a string that will
	be executed by \bsh\b at the specified times.  A percent sign character
	in this field will be translated to a newline character. Any
	character preceded by a backslash (including the %) causes that
	character to be treated literally. Only the first line (up to a "%"
	or end-of-line) of the command field will be executed by the
	command interpreter. The other lines will be made available to the
	command as standard input.]
[+?A job label is generated by a readable hash of the command. If the first
	characters are \b:\b \alabel\a\b;\b then the job label will be set to
	\alabel\a verbatim. Job labels are limited to 12 characters.]
[+?Blank lines and those whose first non-blank character is "#" will
	be ignored.]
[+FILES]{
	[+$HOME/.crontab?Edited per-user crontab entry file.]
}
[+SEE ALSO?\bat\b(1), \bbatch\b(1), \bsh\b(1)]
'
	;;
*)	usage='elr [ file ]'
	;;
esac

# check the options

while	getopts "$usage" OPT
do	case $OPT in
	e)	op=edit ;;
	l)	op=list ;;
	r)	op=remove ;;
	*)	echo "Usage: $command [-elr] [ file ]" >&2
		exit 2
		;;
	esac
done
shift $((OPTIND-1))
case $# in
0)	;;
1)	replace=$1; shift ;;
esac
case $op in
'')	op=replace ;;
*)	case $replace in
	?*)	set -- '-?' ;;
	esac
	;;
esac
case $# in
0)	;;
*)	getopts "$usage" OPT; exit 2 ;;
esac
set -o errexit -o noglob

# check for cron queue access

$cron -a

# do the operation keeping cron entry private

umask 077
case $op in
edit)	trap "rm -f '$temp'" 0 1 2 3 15
	if	test -f "$file"
	then	cat "$file"
	else	print "# MIN HOUR DAY/MONTH MONTH DAY/WEEK COMMAND"
	fi > "$temp"
	${VISUAL:-${EDITOR:-vi}} "$temp"
	;;
list)	if	test -f "$file"
	then	cat "$file"
	fi
	exit
	;;
remove)	$cron -r
	rm -f "$file"
	exit
	;;
replace)trap "rm -f '$temp'" 0 1 2 3 15
	case $replace in
	-)	temp=$file ;;
	"")	cat > "$temp" ;;
	*)	cat "$replace" > "$temp" ;;
	esac
	test -s "$temp"
	;;
esac
exec > /dev/null

# verify the cron entries (no update unless all ok)

while	read -r line
do	set '' $line
	shift
	case $1 in
	''|'#'*)continue ;;
	esac
	case $# in
	[0-5])	print -u2 "$command: $line: invalid entry"
	esac
	$cron -n -t "cron $1 $2 $3 $4 $5" true
done < "$temp"

# update the cron entries

$cron -r
while	read -r line
do	set '' $line
	shift
	case $1 in
	''|'#'*)continue ;;
	esac
	time="$1 $2 $3 $4 $5"
	shift 5
	exec=${*//\\(?)/\1}
	exec=${exec//@([!])%/\1
}
	exec=${exec//^A/\\}
	exec=${exec//@(
*)/\<\<\\
\1
}
	$cron -t "cron $time" "$exec" 2> /dev/null
done < "$temp"
case $replace in
-)	;;
*)	cp "$temp" "$file" ;;
esac
