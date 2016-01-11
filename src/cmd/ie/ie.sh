########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1984-2011 AT&T Intellectual Property          #
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
#                    David Korn <dgkorn@gmail.com>                     #
#                             Pat Sullivan                             #
#                                                                      #
########################################################################
: ie : execute a command with ksh-style input editing

typeset env histfile show command opt prefix dll suffix ifs root path s
typeset OPT ARGV0 USAGE

command=ie
case $(getopts '[-][123:xyz]' opt --xyz 2>/dev/null; echo 0$opt) in
0123)	ARGV0="-a $command"
	USAGE=$'
[-?
@(#)$Id: ie (AT&T Labs Research) 1998-11-30 $
]
'$USAGE_LICENSE$'
[+NAME?ie - execute a command with ksh-style input editing]
[+DESCRIPTION?\bie\b executes a dynamically linked \acommand\a
	with ksh-style input editing on the standard input terminal. The
	default history file name is \b$HOME/.\abase\a, where \abase\a is
	the base name of \acommand\a with leading [\bnox\b]] deleted. All processes
	executed by \acommand\a will have input editing enabled on the same
	history file.]
[h:history?Sets the history file name to \apath\a.]:[path]
[n!:exec?Execute \acommand\a. \b--noexec\b shows how \acommand\a would be
	invoked but does not execute.]
[+DETAILS?Input editing is implemented by intercepting the \bread\b(2) system call
	on file descriptor 0 with a dll that is preloaded at process startup
	before \amain\a() is called. The interception mechanism may involve the
	environment on some systems; in those cases commands like \benv\b(1)
	that clear the enviroment before execution may defeat the \bie\b
	intercept.]
[+?Also intercepted are the \b_\b and \b_libc_\b name permutations of the
	\bread\b call, as well as any 32-bit and 64-bit versions. \bie\b
	ignores calls not present in a particular host system. In addition,
	\bie\b only works on dynamically linked executables that have neither
	set-uid nor set-gid permissions. It may not have the intended effect
	on programs written in a language or linked with a language runtime
	that hides or mangles system call library symbols, or that
	directly emit system call instruction sequences rather than using
	the corresponding library functions, or that dynamically link
	libraries outside of the scope of the \bie\b intercepts.]
[+?Multi-process client-server applications may misbehave if the \bie\b
	environment between the related processes is not kept in sync.]

command [ arg ... ]

[+ENVIRONMENT?\bie\b is implemented by two components: the \bie\b script,
	located on \b$PATH\b and the \bie\b dll (shared library), located
	either on \b$PATH\b or in one of the \b../lib\b* directories on
	\b$PATH\b, depending on local compilation system conventions. Systems
	like \bsgi.mips3\b that support multiple a.out formats may have
	multiple versions of the \bie\b dll. In all cases the \bie\b script
	handles the dll search.]
[+SEE ALSO?\b3d\b(1), \bsh\b(1), \btrace\b(1), \bwarp\b(1), \bread\b(2)]
'
	;;
*)	ARGV0=""
	USAGE='h:[history]nt command [ arg ... ]'
	;;
esac

: grab the options

while	getopts $ARGV0 "$USAGE" OPT
do	case $OPT in
	h)	histfile=$OPTARG ;;
	n)	show=print ;;
	*)	exit 2 ;;
	esac
done
shift $OPTIND-1
case $# in
0)	set -- '-?'; getopts $ARGV0 "$USAGE" OPT; exit 2 ;;
esac

: history file hooks

if	test -x "$1"
then	command=$1
else	command=$(whence $1)
fi
shift
case $histfile in
'')	histfile=${command##*/}
	case $histfile in
	[nox]?*)	histfile=${histfile#?} ;;
	esac
	histfile=$HOME/.$histfile
	;;
esac

: find the dll root dir

ifs=${IFS-'
	 '}
IFS=:
set -A path $PATH
IFS=$ifs
for root in ${path[@]}
do	root=${root%/*}
	set -A lib $root/@(bin|lib)/@(lib|)edit*([0-9]).@(dll|s[ol]*([0-9.]))
	dll=${lib[${#lib[@]}-1]}
	if	test -f $dll
	then	break
	fi
done
prefix=${dll%.@(dll|s[ol]*([0-9.]))}
suffix=${dll##$prefix}
prefix=${prefix#$root/}

: hack the preload magic, and we mean hack

env="HISTFILE='$histfile' LD_PRELOAD='$root/$prefix$suffix${LD_PRELOAD:+ $LD_PRELOAD}'"
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
