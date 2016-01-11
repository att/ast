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
: generate the 3d syscall features
: include sys.tab
#
# this is the worst iffe script, we promise
#
# @(#)syscall.sh (AT&T Research) 2011-12-06
#
eval $1
shell=
( typeset -u u=a && test "A" = "$u" ) >/dev/null 2>&1 && shell=u$shell
( integer n=1+1 && test "2" = "$n" ) >/dev/null 2>&1 && shell=n$shell
( x=a.b.c && test "a" = "${x%%.*}" ) >/dev/null 2>&1 && shell=e$shell
shift
case $1 in
"")	set /dev/null ;;
esac
input=$1
comma=","
nl="
"
tmpsuf="i z m d r f u s n b g t e"
tmpall=
for i in $tmpsuf
do	tmpall=$tmpall$i
done
tmpall="$tmp.[$tmpall]"
rm -f $tmpall
sys=
fun=
stdc=
sym=
vers=
weak=
case $shell in
*n*)	integer inx ;;
esac
inx=0
A_OUTPUT=1
A_INPUT=2
A_ACCESS=3
A_MODE=4
A_NUMBER=5
A_OPEN=6
A_POINTER=7
A_SIZE=8
A_STRING=9
A_VECTOR=A
echo "
#pragma prototyped"
echo "
#if defined(header)
#include \"${tmp}s.h\"
#endif
extern
#if defined(__cplusplus)
\"C\"
#endif
int syscall();
static int ((*i)())=syscall;
main() { return(i==0); }" > $tmp.c

SYS_PREFIX=
SYS_HEADER=
SYS_CALL=
for f in syscall.h sys/syscall.h
do	echo "
#include <$f>
int x;" > ${tmp}s.c
	if	$cc -c ${tmp}s.c </dev/null >/dev/null
	then	SYS_HEADER=$f
		echo "
#include <$f>
" >> $tmp.f
		echo "#include <sys/syscall.h>
#ifdef SYS_exit
int i;
#else
(
#endif" > ${tmp}s.c
		if	$cc -c ${tmp}s.c
		then	l=fun
			echo "int ${l}() { return 0; }" > ${tmp}s.c
			if	$cc -c ${tmp}s.c </dev/null >/dev/null
			then	SYS_CALL=syscall
				s=`nm ${tmp}s.o | sed -e /${l}/!d -e 's/.*[^_]\(_*'$l'[0-9]*\).*/\1/'`
				case $s in
				_*)	SYS_PREFIX=_ ;;
				esac
			fi
		fi
		break
	fi
done

while	:
do	echo '#include <sys/types.h>
#include <dlfcn.h>
#ifdef RTLD_LAZY
main()
{
	dlsym(dlopen("/lib/libc.so", RTLD_LAZY), "open");
	return(0);
}
#else
(
#endif' > ${tmp}h.c
	if	rm -f $tmp.x && {
			$cc -o $tmp.x ${tmp}h.c -ldl </dev/null >/dev/null ||
			$cc -o $tmp.x ${tmp}h.c </dev/null >/dev/null
		} &&
		$executable $tmp.x
	then	sys=dynamic
		echo "#define _dynamic_syscall		1" >> $tmp.m
		lib=
		for d in /lib /usr/lib /shlib /usr/shlib
		do	for s in "*.*" "*.[!a]*"
			do	for b in libc
				do	for i in $d/$b.$s
					do	if	test -f $i
						then	lib=$i
						fi
					done
					case $lib in
					?*)	break 3 ;;
					esac
				done
			done
		done
		case $lib in
		*.[0-9]*.[0-9]*)
			i=`echo $lib | sed 's,\([^0-9]*[0-9]*\).*,\1,`
			if	test -f $i
			then	lib=$i
			fi
			;;
		esac
		# some run time linkers barf with /lib/xxx if
		# /usr/lib/xxx is there
		case $lib in
		/usr*)	;;
		*)	if	test -f /usr$lib
			then	lib=/usr$lib
			fi
			;;
		esac
		case $lib in
		"")	lib=/lib/libc.so.1 ;;
		esac
		echo '#include <sys/types.h>
#include <dlfcn.h>
#ifdef RTLD_NEXT
main()
{
	return 0;
}
#else
(
#endif' > ${tmp}h.c
		if	rm -f $tmp.x && {
			$cc -o $tmp.x ${tmp}h.c -ldl </dev/null >/dev/null ||
			$cc -o $tmp.x ${tmp}h.c </dev/null >/dev/null
			} &&
			$executable $tmp.x
		then	weak=1
		fi
		break
	fi
	if	rm -f $tmp.x &&
		$cc $static -o $tmp.x $tmp.c </dev/null >/dev/null &&
		$executable $tmp.x
	then	case $SYS_HEADER in
		?*)	echo "
#include <$f>
SYS" > ${tmp}s.c
			;;
		esac
		inc="#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:hide syscall
#else
#define syscall        ______syscall
#endif
#include <$SYS_HEADER>
#if defined(__STDPP__directive) && defined(__STDPP__hide)
__STDPP__directive pragma pp:nohide syscall
#else
#undef  syscall
#endif"
		{
		case $SYS_HEADER in
		?*)	echo "$inc" ;;
		*)	echo "#define SYS_exit 1" ;;
		esac
		echo "main() { syscall(SYS_exit, 0); exit(1); }"
		} > ${tmp}m.c
		if	rm -f ${tmp}m.x &&
			$cc $static -o ${tmp}m.x ${tmp}m.c </dev/null >/dev/null &&
			./${tmp}m.x
		then	case $SYS_HEADER in
			?*)	echo "$inc" ;;
			esac
			rm -f $tmp.x
			if	rm -f $tmp.x &&
				$cc -Dsyscall=swapon $static -o $tmp.x $tmp.c </dev/null >/dev/null &&
				$executable $tmp.x
			then	sys=bsd
			else	sys=att
			fi
			echo "#define _static_syscall		1" >> $tmp.m
			break
		fi
	fi
	sys=mangle
	echo "#define _mangle_syscall		1" >> $tmp.m
	if	LD_PRELOAD=/dev/null ${_ld_preload_test-cat} </dev/null >/dev/null
	then	echo "
static int _lib_mangle_lib_;" >> $tmp.m
	fi
	break
done
echo iffe: test: type=$sys header=$SYS_HEADER ${SYS_CALL:+call=$SYS_CALL} ${SYS_PREFIX:+prefix=$SYS_PREFIX} >&$stderr
echo "#if SYSTRACE3D

/* output */

#define A_OUTPUT	0x$A_OUTPUT

/* input only */

#define A_INPUT		0x$A_INPUT
#define A_ACCESS	0x$A_ACCESS
#define A_MODE		0x$A_MODE
#define A_NUMBER	0x$A_NUMBER
#define A_OPEN		0x$A_OPEN
#define A_POINTER	0x$A_POINTER
#define A_SIZE		0x$A_SIZE
#define A_STRING	0x$A_STRING
#define A_VECTOR	0x$A_VECTOR

typedef struct
{
	Sysfunc_t	func;
	short		index;
	short		nov;
	unsigned short	call;
	unsigned short	args;
	const char*	name;
	unsigned char	type[8];
	unsigned long	count;
	unsigned long	megs;
	unsigned long	units;
	Sysfunc_t	last;
	unsigned short	active;
} Systrace_t;
" > $tmp.b

echo "static Systrace_t	sys_trace[] =
{
" > $tmp.t
case $sys in
*)	SYSCALL3D=1 ;;
esac
for i in $cc
do	case $i in
	-D*)	case $shell in
		*e*)	v=${i#-D}
			v=${v%%=*}
			;;
		*)	v=`echo '' $i | sed -e 's/^ -D//' -e 's/=.*//'`
			;;
		esac
		eval $v=1
		;;
	esac
done
exec <$input
ifi=0
ifs=${IFS-'
	 '}
while	:
do	IFS="	"
	read line || break
	set $line
	IFS=$ifs
	case $1 in
	\#*)	continue ;;
	"")	break ;;
	"if")	shift
		eval $*
		case $? in
		0)	ifi="0 $ifi" ;;
		*)	ifi="1 $ifi" ;;
		esac
		continue
		;;
	"elif")	case $ifi in
		0)	echo "$0: no if for elif" >&$stderr
			break
			;;
		esac
		case $ifi in
		1*)	shift
			eval $*
			case $? in
			0)	;;
			*)	continue ;;
			esac
			i=0
			;;
		*)	i=2
			;;
		esac
		set $ifi
		shift
		ifi="$i $ifi"
		;;
	"else")	case $ifi in
		0)	echo "$0: no if for else" >&$stderr
			break
			;;
		esac
		case $ifi in
		1*)	i=0 ;;
		*)	i=2 ;;
		esac
		set $ifi
		shift
		ifi="$i $ifi"
		;;
	"fi")	case $ifi in
		0)	echo "$0: no if for fi" >&$stderr
			break
			;;
		esac
		set $ifi
		shift
		ifi=$*
		continue
		;;
	esac
	case $ifi in
	0*)	;;
	*)	continue ;;
	esac
	l=$1
	q=$2
	r=$3
	p=$4
	shift 4
	echo iffe: test: system call $l >&$stderr
	case $r in
	"bool")	cast="(" tsac="<0?-1:0)" r=int ;;
	"int")	cast= tsac= ;;
	"DIR*")	cast="(DIR$r)" tsac= ;;
	*)	cast="($r)" tsac= ;;
	esac
	case $shell in
	*u*)	typeset -u u=$l ;;
	*)	u=`echo $l | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ;;
	esac
	w=$l
	case $l in
	_*)	case $shell in
		*e*)	v=${u#?} ;;
		*)	v=`echo $u | sed 's/.//'` ;;
		esac
		echo "#define $v			$u" >> $tmp.r
		m=$l
		echo "#undef	$l	/*MANGLENAME*/" >> $tmp.z
		;;
	*)	U=$u
		m=_$l
		for i in __libc_ _ __
		do	if	rm -f $tmp.x &&
				$cc -Dsyscall=${i}$l $static -o $tmp.x $tmp.c </dev/null >/dev/null &&
				$executable $tmp.x
			then	case $sys:$l in
				*:creat|*:creat[0-9]*)	# sol6.sun4
					;;
				*:open|*:open[0-9]*)	# sol6.sun4
					;;
				dynamic:*)
					# test if the _ version is a weak ref
					echo "#include <sys/types.h>
#include <dlfcn.h>
main()
{
	void*	p;
	void*	u;
	void*	r;
	p = dlopen(\"$lib\", 1);
	u = dlsym(p, \"${i}$l\");
	r = dlsym(p, \"$l\");
	return(u == r);
}" > ${tmp}h.c
					if	rm -f $tmp.x && {
							$cc -o $tmp.x ${tmp}h.c -ldl </dev/null >/dev/null ||
							$cc -o $tmp.x ${tmp}h.c </dev/null >/dev/null
						} &&
						./$tmp.x
					then	break
					fi
					;;
				esac
				w=${i}$l
				case $shell in
				*u*)	typeset -u I=$i ;;
				*)	I=`echo $i | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ;;
				esac
				{
				echo "#undef  $u"
				echo "#define $u			${I}$u"
				} >> $tmp.r
				u=${I}$u
				break
			fi
		done
		{
		echo "#undef	$l	/*MANGLENAME*/"
		case $w in
		$l)	;;
		*)	echo "#undef	$w" ;;
		esac
		} >> $tmp.z
		;;
	esac
	case $p in
	void)		b=0 a= ;;
	*,*,*,*,*,*)	b=6 a=a,b,c,d,e,f ;;
	*,*,*,*,*)	b=5 a=a,b,c,d,e ;;
	*,*,*,*)	b=4 a=a,b,c,d ;;
	*,*,*)		b=3 a=a,b,c ;;
	*,*)		b=2 a=a,b ;;
	*)		b=1 a=a ;;
	esac
	org=$p
	A=$a
	B=$b
	alt=
	def=
	inc=
	ind=
	map=
	num=
	opt=
	ref=
	ver=
	VER=
	while	:
	do	case $1 in
		alt*)	alt=$2
			shift 2
			;;
		num*)	shift
			;;
		var*)	var=$2
			shift 2
			set sys/types.h $1
			{
			for i
			do	echo "#include <$i>"
			done
			echo "#ifdef __STDC__
extern $r $l($var);
#endif"
			} > ${tmp}h.c
			if	$cc -c ${tmp}h.c </dev/null >/dev/null
			then	inc="$inc $*"
				p=$var
				case $p in
				void)		b=0 a= ;;
				*,*,*,*,*,*)	b=6 a=a,b,c,d,e,f ;;
				*,*,*,*,*)	b=5 a=a,b,c,d,e ;;
				*,*,*,*)	b=4 a=a,b,c,d ;;
				*,*,*)		b=3 a=a,b,c ;;
				*,*)		b=2 a=a,b ;;
				*)		b=1 a=a ;;
				esac
				org=$p
				A=$a
				B=$b
			fi
			echo "#define ARG3D$m		$B" >> $tmp.f
			break
			;;
		ver*)	VER=1
			case $2 in
			-)	ref= ;;
			*)	ref=$2 ;;
			esac
			shift 2
			case $l in
			[fl]*)	case $shell in
				*u*)	j=${l#?}
					i=${l%$j}
					xxx=_${i}x${j}
					;;
				*)	xxx=`echo $l | sed -e 's/\(.\)\(.*\)/_\1x\2/'`
					;;
				esac
				;;
			*)	xxx=_x${l}
				;;
			esac
			for i in sys/types.h $ref
			do	echo "#include <$i>"
			done > ${tmp}h.c
			i=
			for j in `$cc -E ${tmp}h.c | sed -e '/^#[line 	]*1[ 	]/!d' -e 's/.*"\(.*\)".*/\1/'`
			do	test -f "$j" && i="$i $j"
			done
			ver=`sed -e "/define[ 	]*[A-Z_0-9]*64_VER/!d" -e 's/.*define[ 	]*\([A-Z_0-9][A-Z_0-9]*\).*/\1/' $i </dev/null | sort -u`
			ver64="$ver64 $ver"
			ver=`sed -e "/${xxx}.*_VER/!d" -e 's/.*([ 	]*\([A-Z_0-9][A-Z_0-9]*\).*/\1/' $i </dev/null | sort -u`
			case $l:$ver in
			*:*STAT*)
				;;
			*stat:*)for j in _SCO_STAT_VER
				do if grep '[ 	]_SCO_STAT_VER[ 	]' $i >/dev/null
				   then	ver=$j; break
				   fi
				done
				;;
			esac
			if	grep "[^a-zA-Z_0-9]_$xxx" $i >/dev/null
			then	xxx=_$xxx
			fi
			;;
		*.h)	inc="$inc $1"
			shift
			;;
		[0-9]*)	case $sys in
			bsd)	case $2 in
				[0-9]*)	num=$2 ;;
				*)	num=$1 ;;
				esac
				;;
			*)	num=$1
				;;
			esac
			case $2 in
			[0-9]*)	shift 2 ;;
			*)	shift ;;
			esac
			;;
		"")	break
			;;
		*)	case $# in
			[012])	break ;;
			*)	shift 2 ;;
			esac
			;;
		esac
	done
	case $num:$sys in
	:dynamic)	num=1 ;;
	esac
	call=
	case $inc in
	?*)	for i in sys/types.h $inc
		do	echo "#include <$i>"
		done > ${tmp}s.h
		opt="$opt -Dheader"
		;;
	esac
	case $VER:$ver in
	*:?*)	if	rm -f $tmp.x &&
			$cc -Dsyscall=$xxx $static -o $tmp.x $tmp.c </dev/null >/dev/null &&
			$executable $tmp.x
		then	# hack test for last arg indirect in xxx version
			case $stdc in
			"")	{
				echo "extern int foo(const char*);"
				echo "extern int foo(const char**);"
				} > ${tmp}h.c
				if	$cc -c ${tmp}h.c </dev/null >/dev/null
				then	stdc=0
				else	stdc=1
				fi
				;;
			esac
			ra=$a
			va=$a
			case $stdc in
			1)	{
				for i in sys/types.h $ref
				do	echo "#include <$i>"
				done
				echo "#ifdef INDIRECT"
				echo "#define REFERENCE *"
				echo "#else"
				echo "#define REFERENCE"
				echo "#endif"
				echo "extern $r $xxx(const int, $p REFERENCE);"
				} > ${tmp}h.c
				if	$cc -c ${tmp}h.c </dev/null >/dev/null
				then	: ok
				elif	$cc -c -DINDIRECT ${tmp}h.c </dev/null >/dev/null
				then	ind="*"
					case $shell in
					*e*)	i=${a%?}
						ra="${i}&${a#$i}"
						va="${i}*${a#$i}"
						;;
					*)	ra=`echo $a | sed -e 's/\(.\)$/\&\1'`
						va=`echo $a | sed -e 's/\(.\)$/\*\1'`
						;;
					esac
				fi
				;;
			esac
			case $shell in
			*u*)	typeset -u y=$xxx ;;
			*)	y=`echo $xxx | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ;;
			esac
			case $sys in
			dynamic|mangle)
				echo "extern $r		$y(const int, $p$ind);" >> $tmp.e
				;;
			esac
			{
			echo "#define SYS3D$m		$inx"
			echo "#ifndef SYS$m"
			echo "#define SYS$m		(-1)"
			echo "#endif"
			echo "#ifndef SYS$xxx"
			echo "#define SYS$xxx		(-1)"
			echo "#endif"
			echo "#define SYSNOV$xxx	SYS$m"
			} >> $tmp.f
			case $shell in
			*n*)	((inx=inx+1)) ;;
			*)	inx=`expr $inx + 1` ;;
			esac
			case $ver in
			*[mM][kK][nN][oO][dD]*)	verid=_MKNOD_VER ;;
			*[sS][tT][aA][tT]*)	verid=_STAT_VER ;;
			*)			verid=$ver ;;
			esac
			{
			case $y in
			__*)	case $shell in
				*e*)	Y=${y#__} ;;
				*)	Y=`echo $y | sed -e 's/__//'` ;;
				esac
				echo "#undef  $Y"
				echo "#define $Y	$y"
				;;
			_*)	case $shell in
				*e*)	Y=${y#_} ;;
				*)	Y=`echo $y | sed -e 's/_//'` ;;
				esac
				echo "#undef  $Y"
				echo "#define $Y	$y"
				;;
			esac
			echo "#undef  $u"
			echo "#define $u($a)	$y(_3D${verid}${ra:+,}$ra)"
			echo "#undef  $y"
			echo "#define $y(v${a:+,}$a)		${cast}SYSCALL(SYS3D$m,v${a:+,}$a)${tsac}"
			} >> $tmp.u
			case $p in
			void)	c=1
				set $r int
				;;
			*)	IFS=$comma
				set int${comma}$p
				c=$#
				set $r${comma}int${comma}$p$ind
				IFS=$ifs
				;;
			esac
			d=
			s=
			for i in 0 1 2 3 4 5
			do	case $# in
				0)	break ;;
				esac
				case $1 in
				*"mode_t")
					d=${d}${s}A_MODE
					;;
				*"size_t")
					d=${d}${s}A_SIZE
					;;
				*"char* const*"|*"char**")
					d=${d}${s}A_VECTOR
					;;
				*"char*")
					case $r:$2 in
					*"ssize_t":*"size_t")
						d=${d}${s}A_OUTPUT,A_SIZE
						shift
						;;
					*)	d=${d}${s}A_STRING
						;;
					esac
					;;
				*"const void*")
					d=${d}${s}A_INPUT
					;;
				*"void*")
					d=${d}${s}A_OUTPUT
					;;
				*"struct stat*"|*"int*")
					d=${d}${s}A_OUTPUT
					;;
				*"...")	case $l in
					*open|*open64)	d=${d}${s}A_MODE ;;
					*)		d=${d}${s}A_POINTER ;;
					esac
					;;
				*"[]"|*"*")
					d=${d}${s}A_POINTER
					;;
				*)	d=${d}${s}A_NUMBER
					;;
				esac
				s=,
				shift
			done
			case $sys in
			dynamic)x="0, "
				;;
			mangle)	x="$u, "
				echo "#undef $u
extern long $u();" >> $tmp.g
				;;
			*)	x=
				;;
			esac
			echo iffe: test: $l "=>" $xxx >&$stderr
			echo "{ ${x}SYS$xxx, SYSNOV$xxx, MSG_CALL(MSG_$q), $c, \"$xxx\", { $d } }," >> $tmp.t
			# NOTE: no varargs here
			IFS=$comma
			set $p
			IFS=$ifs
			d=
			f=
			o=
			for i in a b c d e f
			do	case $# in
				0)	break ;;
				1)	e=$ind ;;
				*)	e= ;;
				esac
				case $1 in
				*\[\])	case $shell in
					*e*)	k=${1%\[\]} ;;
					*)	k=`echo $1 | sed 's/\[\]\$//'` ;;
					esac
					i="$i"'[]'
					;;
				*)	k=$1
					;;
				esac
				case $k in
				*char*)	n=path ;;
				*dev_t)	n=dev ;;
				*int*)	n=fd ;;
				*mode_t*)n=mode ;;
				*stat*)	n=st ;;
				*)	n="<$k>${i}" ;;
				esac
				case $f in
				'')	f=$n ;;
				*)	f=$f,$n ;;
				esac
				case $d in
				"")	d="$k$e $n"
					o="$k$e $n;"
					;;
				*)	d="$d, $k$e $n"
					o="$o $k$e $n;"
					;;
				esac
				shift
			done
			echo "#define ${U}3D		_3d_$m" >> $tmp.d
			echo "#define ${l}3d($a)	$xxx __PARAM__((const int _3d_ver, $d),(_3d_ver${f:+,} $f)) __OTORP__(int _3d_ver; $o)" >> $tmp.d
			eval v='$'_INIT$ver
			case $v in
			"")	eval _INIT$ver=1
				vers="$vers $ver"
				;;
			esac
			case $sym in
			"")	sym="$xxx" fun="$xxx()" ;;
			*)	sym="$sym,$nl	$xxx" fun="$fun,$nl	$xxx()" ;;
			esac
			eval _SYS_${l}_VER=_3D$verid
			continue
		fi
		;;
	1:*)	case $SYS_CALL in
		?*)	echo "#include <${SYS_HEADER}>
#ifdef SYS_${l}
(
#endif
#include <sys/types.h>
#include <sys/stat.h>
extern int ${l}();
int fun() { struct stat st; return ${l}(0,&st); }" > ${tmp}s.c
			if	$cc -c ${tmp}s.c >/dev/null 2>&1
			then	s=`nm ${tmp}s.o | sed -e /$l/!d -e 's/.*[^_]\('${SYS_PREFIX}'_*'${l}'[0-9]*\).*/\1/'`
				case $s in
				$l)	;;
				*)	echo "#include <sys/syscall.h>
#ifndef SYS_${s}
(
#else
int i;
#endif" > ${tmp}s.c
					if	$cc -c ${tmp}s.c >/dev/null 2>&1
					then	def=1
						map=$s
						echo "#define SYS_${l}	SYS_${s}" >> $tmp.f
						echo iffe: test: $l "=>" $s >&$stderr
					fi
					;;
				esac
			else	def=1
			fi
			;;
		esac
		;;
	esac
	if	rm -f $tmp.x &&
		$cc -Dsyscall=$l $opt $static -o $tmp.x $tmp.c </dev/null >/dev/null &&
		$executable $tmp.x
	then	case $sys in
		att|bsd|dynamic|mangle)
			case $SYS_HEADER in
			"")	case $num in
				"")	;;
				0)	echo "#define $u		$l" >> $tmp.r
					;;
				*)	case $sys in
					att|bsd)	echo "#define SYS$m		$num" >> $tmp.s ;;
					esac
					call=sys
					;;
				esac
				;;
			*)	case `$cc -E -DSYS=SYS$m ${tmp}s.c </dev/null | grep SYS$m` in
				"")	call=sys
					;;
				*)	case $num in
					[0-9]*)	call=nosys ;;
					*)	call=nolib ;;
					esac
					;;
				esac
				;;
			esac
			;;
		*)	echo "#define _error_$u		1" >> $tmp.m
			;;
		esac
	else	case $num in
		[0-9]*)	call=nosys ;;
		*)	call=nolib ;;
		esac
	fi
	case $call in
	nolib|nosys)
		{
		echo "#ifndef SYS$m"
		echo "#define SYS$m	(-1)"
		echo "#endif"
		} >> $tmp.f
		;;
	*)	case $def:$SYS_HEADER in
		:?*)	echo "#include <${SYS_HEADER}>
#ifdef SYS${m}
(
#else
int i;
#endif" > ${tmp}s.c
			if	$cc -c ${tmp}s.c >/dev/null 2>&1
			then	echo "#define SYS$m		(-1)" >> $tmp.f
			fi
			;;
		esac
		;;
	esac
	case $call in
	nolib|nosys)
		echo "#define _nosys$m		1" >> $tmp.u
		;;
	esac
	case $call in
	nolib)	;;
	nosys)	case $alt in
		"")	
			echo "#define $u($a)		NOSYS()" >> $tmp.u
			echo "#define _stub_$w		1" >> $tmp.n
			w=_no_$w
			;;
		*)	case $shell in
			*u*)	typeset -u y=$alt ;;
			*)	y=`echo $alt | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ` ;;
			esac
			echo "#define $u			$y" >> $tmp.r
			;;
		esac
		;;
	sys)	{
		echo "#define SYS3D$m		$inx"
		case $sys in
		dynamic|mangle)
			echo "#ifndef SYS$m"
			echo "#define SYS$m		(-1)"
			echo "#endif"
			;;
		esac
		} >> $tmp.f
		case $shell in
		*n*)	((inx=inx+1)) ;;
		*)	inx=`expr $inx + 1` ;;
		esac
		{
		echo "#undef  $u"
		echo "#define $u($a)		${cast}SYSCALL(SYS3D$m${A:+,}$A)${tsac}"
		} >> $tmp.u
		case $p in
		void)	set $r
			c=0
			;;
		*)	IFS=$comma
			set $p
			c=$#
			set $r${comma}$p
			IFS=$ifs
			;;
		esac
		d=
		s=
		for i in 0 1 2 3 4 5
		do	case $# in
			0)	break ;;
			esac
			case $1 in
			*"mode_t")
				d=${d}${s}A_MODE
				;;
			*"size_t")
				d=${d}${s}A_SIZE
				;;
			*"char* const*"|*"char**")
				d=${d}${s}A_VECTOR
				;;
			*"char*")
				case $r:$2 in
				*"ssize_t":*"size_t")
					d=${d}${s}A_OUTPUT,A_SIZE
					shift
					;;
				*)	d=${d}${s}A_STRING
					;;
				esac
				;;
			*"const void*")
				d=${d}${s}A_INPUT
				;;
			*"void*")
				d=${d}${s}A_OUTPUT
				;;
			*"struct stat*"|*"int*")
				d=${d}${s}A_OUTPUT
				;;
			*"...")	case $l in
				*open)	d=${d}${s}A_MODE ;;
				*)	d=${d}${s}A_POINTER ;;
				esac
				;;
			*"[]"|*"*")
				d=${d}${s}A_POINTER
				;;
			*)	d=${d}${s}A_NUMBER
				;;
			esac
			s=,
			shift
		done
		case $sys in
		mangle)	x="$u, "
			echo "#undef $u
extern long $u();" >> $tmp.g
			;;
		*)	x="0, "
			;;
		esac
		i=$l
		case $i in
		_exit)	{
			echo "static int hit;"
			echo "void _exit(code) int code; { if (hit++) abort(); __exit(code); }"
			echo "main() { return 0; }"
			} > ${tmp}h.c
			$cc -o ${tmp}h.x ${tmp}h.c && ./${tmp}h.x && i=_$i
			;;
		*)	case $map:$weak in
			?*:*)	i=$map ;;
			:?*)	i=$w ;;
			esac
			;;
		esac
		echo "{ ${x}SYS$m, (-1), MSG_CALL(MSG_$q), $c, \"$i\", { $d } }," >> $tmp.t
		;;
	*)	case $num in
		?*)	echo "extern $r			$u($p);" >> $tmp.e ;;
		esac
		;;
	esac
	i=
	case $call in
	nolib)	;;
	nosys)	;;
	*)	case $l:$w in
		_*)	;;
		*:_*)	i=1 ;;
		esac
		;;
	esac
	case $i in
	"")	echo "#define ${l}3d		$w" >> $tmp.d
		;;
	*)	case $p in
		void)	d=void o= va= vo=
			;;
		*)	IFS=$comma
			set $p
			IFS=$ifs
			d=
			o=
			va=$a
			vo=
			vx=
			for i in a b c d e f
			do	case $# in
				0)	break ;;
				esac
				case $1 in
				*...)	d="$d, ..."
					va=va_alist
					case $l in
					fcntl)	t='void*' ;;
					*)	t=int ;;
					esac
					vo="__OTORP__($o) $t $i; va_list ap; __VA_START__(ap,$vx); __OTORP__("
					o=va_dcl
					IFS=$comma
					set $p
					IFS=$ifs
					for i in a b c d e f
					do	case $1 in
						*...)	break ;;
						esac
						vo="$vo$i = va_arg(ap, $1);"
						shift
					done
					case $l in
					fcntl)	vo="$vo) $i = va_arg(ap, void*); va_end(ap);" ;;
					*)	vo="$vo) $i = va_arg(ap, int); va_end(ap);" ;;
					esac
					break
					;;
				*\[\])	case $shell in
					*e*)	k=${1%\[\]} ;;
					*)	k=`echo $1 | sed 's/\[\]\$//'` ;;
					esac
					i="$i"'[]'
					;;
				*)	k=$1
					;;
				esac
				vx=$i
				case $d in
				"")	d="$k $i"
					o="$k $i;"
					;;
				*)	d="$d, $k $i"
					o="$o $k $i;"
					;;
				esac
				shift
			done
			;;
		esac
		case $o in
		?*)	o="__OTORP__($o) " ;;
		esac
		case $r in
		void)	ft='*' fr= ;;
		*)	ft= fr='return ' ;;
		esac
		z="$r $w"
		echo "#define ${U}3D		${z}" >> $tmp.d
		W=
		case $w in
		__libc_*) W=" $r __$l __PARAM__(($d),($va)) $o{$vo $fr$w($a); }" ;;
		__*) W=" $r _$l __PARAM__(($d),($va)) $o{$vo $fr$w($a); }" ;;
		esac
		echo "#define ${l}3d		${ft}_3d_stub${m} = 0; extern __MANGLE__ $r $w __PROTO__(($p)); $r $l __PARAM__(($d),($va)) $o{$vo $fr$w($a); }$W $z" >> $tmp.d
		;;
	esac
	case $call:$sym in
	nolib:*);;
	nosys:*);;
	*:)	sym="$w" fun="$w()" ;;
	*)	sym="$sym,$nl	$w" fun="$fun,$nl	$w()" ;;
	esac
done
IFS=$ifs
if	test -f $tmp.t
then	{
	echo "};
"
	case $sys in
	dynamic|mangle)
		echo "#define SYSCALL3D(z,a,b,c,d,e,f)	(*sys_trace[z].func)(a,b,c,d,e,f)"
		case $sys in
		dynamic|mangle)
			echo "#undef	fstat"
			case $_SYS_fstat_VER in
			"")	echo "#define fstat(a,b)	SYSCALL3D(SYS3D_fstat,a,b,0,0,0,0)" ;;
			*)	echo "#define fstat(a,b)	SYSCALL3D(SYS3D_fstat,$_SYS_fstat_VER,a,b,0,0,0)" ;;
			esac
			echo "#undef	write"
			echo "#define write(a,b,c)	SYSCALL3D(SYS3D_write,a,b,c,0,0,0)"
			;;
		esac
		;;
	esac
echo "
#else
"
echo "
#define SYSCALL		syscall3d

extern long	syscall3d(int, ...);

#endif"
	} >> $tmp.t
else	echo "#undef	SYSTRACE3D" > $tmp.t
fi
echo "
#endif" >> $tmp.e
echo "
#ifdef	SYSENT3D

extern int
	$fun;

int			(*_3d_sysent[])() = {
	$sym
};

#else

extern int		(*_3d_sysent[])();
#define PULL_IN_3D		int (*pull_in_3d)() = _3d_sysent[0];
" >> $tmp.m
for ver in $vers
do	vercall=
	case $ver in
	*[mM][kK][nN][oO][dD]*)	vercall=MKNOD ;;
	*[sS][tT][aA][tT]*)	vercall=STAT ;;
	esac
	case $vercall in
	?*)	echo "#define _3D_${vercall}_VER	${ver}"
		for alt in $ver64
		do	case $alt in
			*${vercall}64*) echo "#define _3D_${vercall}64_VER	${alt}" ;;
			esac
		done
		;;
	esac
done >> $tmp.f
for i in $tmpsuf
do	if	test -f $tmp.$i
	then	echo
		cat $tmp.$i
	fi
done
echo
