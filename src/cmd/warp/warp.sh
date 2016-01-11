########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1998-2011 AT&T Intellectual Property          #
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
: warp : execute a command in a different time frame

integer warp now base factor
typeset env show command opt prefix dll suffix ifs root path s
typeset base_str
typeset OPT ARGV0 USAGE

command=warp
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: warp (AT&T Labs Research) 1999-11-19 $
]
'$USAGE_LICENSE$'
[+NAME?warp - execute a command in a different time frame]
[+DESCRIPTION?\bwarp\b executes a dynamically linked \acommand\a
	in a different time frame by intercepting time related system calls
	and modifying the times seen by \acommand\a using the formula:]{
	[+time = time + warp + (time - base) * (factor - 1)?]
}
[+?where \awarp\a is \adate-now\a, \abase\a is \adate\a by default, and
	\afactor\a is 1 by default. Command argument date specifications
	support common conventions:]{
	[+yesterday?]
	[+next week?]
	[+50 days?]
	[+2000-02-28+00:00?]
	[+feb 28 2000?]
	[+#\aseconds\a?A \btime_t\b value in \aseconds\a since the epoch.]
}
[b:base?Set the base or starting date to \adate\a. Useful for repeating
	a set of tests.]:[date:=date]
[f:factor?Set the warped clock to tick at \afactor\a seconds per real second.]:
	[factor:=1]
[n!:exec?Execute \acommand\a. \b--noexec\b shows how \acommand\a would be
	invoked but does not execute.]
[t:trace?Trace each intercepted system call on the standard error.]
[+DETAILS?\bwarp\b executes \acommand\a with optional \aargs\a, or \bsh\b(1) if
	\acommand\a is omitted. All processes executed by \acommand\a are
	warped in the same time frame. Time progresses for \acommand\a and its
	children at the rate of \afactor\a times the system clock. Any files
	created by \acommand\a or its children will appear newer than \adate\a
	to \acommand\a and its children, but will actually be in the normal
	time frame for non-warped commands.]
[+?Processes are warped by intercepting system calls with a dll that is
	preloaded at process startup before \amain\a() is called. The
	interception mechanism may involve the environment on some systems;
	in those cases commands like \benv\b(1) that clear the enviroment
	before execution may defeat the \bwarp\b intercepts. The intercepted
	system calls are:]{
	[+alarm?]
	[+fstat?]
	[+getitimer?]
	[+gettimeofday?]
	[+lstat?]
	[+poll?]
	[+select?]
	[+setitimer?]
	[+stat?]
	[+time?]
	[+times?]
	[+utime?]
	[+utimes?]
}
[+?Also intercepted are the \b_\b and \b_libc_\b name permutations of the
	calls, as well as any 32-bit and 64-bit versions, and the abominable
	System V \bx\b versions of the \bstat\b(2) family. \bwarp\b ignores
	calls not present in a particular host system. In addition, \bwarp\b
	only works on dynamically linked executables that have neither set-uid
	set-uid nor set-gid permissions. It may not have the intended effect
	on programs written in a language or linked with a language runtime
	that hides or mangles system call library symbols, or that
	directly emit system call instruction sequences rather than using
	the corresponding library functions, or that dynamically link
	libraries outside of the scope of the \bwarp\b intercepts.]
[+?Multi-process client-server applications may misbehave if the \bwarp\b
	environment between the related processes is not kept in sync.]

date [ command [ arg ... ] ]

[+ENVIRONMENT?\bwarp\b is implemented by three components: the \bwarp\b script,
	located on \b$PATH\b; the \bwarp\b dll (shared library), located either
	on \b$PATH\b or in one of the \b../lib\b* directories on \b$PATH\b,
	depending on local compilation system conventions; and the \bast\b
	\bdate\b(1) command, located on \b$PATH\b, that supports conversion
	to/from \btime_t\b values at the shell level. Systems like
	\bsgi.mips3\b that support multiple a.out formats may have multiple
	versions of the \bwarp\b dll. In all cases the \bwarp\b script handles
	the dll search.]
[+EXAMPLES?]{
	[+$ date -f %K?1998-03-11+13:41]
	[+$ warp 2000-02-29+12::30::30 date -f %K?2000-02-29+12:30]
	[+$ warp "2 years" date -f %K?2000-01-01+00:00]
	[+$ PS1="(warp) " warp -f $((60*60*24)) 2000-02-29+12::30::30?#
		interactive \bsh\b(1) where 1 warped day passes for each
		real second.]
}
[+SEE ALSO?\b3d\b(1), \bdate\b(1), \benv\b(1), \bie\b(1), \bsh\b(1),
	\btrace\b(1), \bstat\b(2)]
'
	;;
*)	ARGV0=""
	USAGE='b:[base]f:[factor]nt date [ command [ arg ... ] ]'
	;;
esac

: grab the options

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	b)	base_str=$OPTARG ;;
	f)	factor=$OPTARG ;;
	n)	show=print ;;
	t)	opt="$opt-$OPT " ;;
	*)	exit 2 ;;
	esac
done
shift $OPTIND-1
case $# in
0)	set -- '-?'; getopts $ARGV0 "$USAGE" OPT; exit 2 ;;
esac

: find the warp dll root dir

ifs=${IFS-'
	 '}
IFS=:
set -A path $PATH
IFS=$ifs
for root in ${path[@]}
do	root=${root%/*}
	set -A lib $root/@(bin|lib)/@(lib|)warp*([0-9]).@(dll|dylib|s[ol]*([0-9.]))
	dll=${lib[${#lib[@]}-1]}
	if	test -f $dll
	then	break
	fi
done
prefix=${dll%.@(dll|dylib|s[ol]*([0-9.]))}
suffix=${dll##$prefix}
prefix=${prefix#$root/}

: determine the warp offset

(( now = $(LD_LIBRARY_PATH=$root/lib:$LD_LIBRARY_PATH $root/bin/date -f %s) ))
case $1 in
now)	(( warp = now ))
	;;
+([0-9]))
	(( warp = $1 ))
	;;
*)	(( warp = $(LD_LIBRARY_PATH=$root/lib:$LD_LIBRARY_PATH $root/bin/date -f %s -s "$1") ))
	;;
esac
shift

: determine the warp factor base

if	(( factor ))
then	case $base_str in
	''|now)	(( base = now ))
		;;
	+([0-9]))
		(( base = $base_str ))
		;;
	*)	(( base = $(LD_LIBRARY_PATH=$root/lib:$LD_LIBRARY_PATH $root/bin/date -f %s -s "$base_str") ))
		;;
	esac
	opt="$opt-b$base -f$factor "
fi

: final adjustments

(( warp = warp - now ))

: if no command then start a warped shell

case $# in
0)	command=${SHELL:-ksh} ;;
*)	if	test -x "$1"
	then	command=$1
	else	command=$(whence $1)
	fi
	shift
	;;
esac

: hack the preload magic, and we mean hack

env="WARP='$opt$warp' LD_PRELOAD='$root/$prefix$suffix${LD_PRELOAD:+ $LD_PRELOAD}' DYLD_INSERT_LIBRARIES='$root/$prefix$suffix${DYLD_INSERT_LIBRARIES:+ $DYLD_INSERT_LIBRARIES}'" 
case $root in
*-n32)	env="$env _RLDN32_LIST=$root/$prefix$suffix"
	case $_RLDN32_LIST in
	"")	env="$env:DEFAULT" ;;
	*)	env="$env:$_RLDN32_LIST" ;;
	esac
	;;
*)	if	test -f $root/$prefix-n32$suffix
	then	env="$env _RLD64_LIST=$dll"
		case $_RLD64_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLD64_LIST" ;;
		esac
		env="$env _RLDN32_LIST=$root/$prefix-n32$suffix"
		case $_RLDN32_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLDN32_LIST" ;;
		esac
		env="$env _RLD_LIST=$root/$prefix-o32$suffix"
		case $_RLD_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLD_LIST" ;;
		esac
	elif	test -f $root/$prefix-64$suffix -o -f $root/$prefix-o32$suffix
	then	env="$env _RLD64_LIST=$root/$prefix-64$suffix"
		case $_RLD64_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLD64_LIST" ;;
		esac
		env="$env _RLDN32_LIST=$dll"
		case $_RLDN32_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLDN32_LIST" ;;
		esac
		env="$env _RLD_LIST=$root/$prefix-o32$suffix"
		case $_RLD_LIST in
		"")	env="$env:DEFAULT" ;;
		*)	env="$env:$_RLD_LIST" ;;
		esac
	else	case $root in
		*/sgi.mips[2-9]*)
			root=${root%-*}
			root=${root%?}
			env="$env _RLD64_LIST=${root}4/$prefix$suffix"
			case $_RLD64_LIST in
			"")	env="$env:DEFAULT" ;;
			*)	env="$env:$_RLD64_LIST" ;;
			esac
			env="$env _RLDN32_LIST=${root}3/$prefix$suffix"
			case $_RLDN32_LIST in
			"")	env="$env:DEFAULT" ;;
			*)	env="$env:$_RLDN32_LIST" ;;
			esac
			env="$env _RLD_LIST=${root}2/$prefix$suffix"
			case $_RLD_LIST in
			"")	env="$env:DEFAULT" ;;
			*)	env="$env:$_RLD_LIST" ;;
			esac
			;;
		*)	env="$env _RLD_LIST=${root}/$prefix$suffix"
			case $_RLD_LIST in
			"")	env="$env:DEFAULT" ;;
			*)	env="$env:$_RLD_LIST" ;;
			esac
		esac
	fi
	;;
esac

: engage

case $show in
"")	eval $env '$command "$@"' ;;
*)	$show $env $command "$@" ;;
esac
