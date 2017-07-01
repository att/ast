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
#                     Phong Vo <phongvo@gmail.com>                     #
#              Doug McIlroy <doug@research.bell-labs.com>              #
#                                                                      #
########################################################################
: sorttest [ sort test-options ... ]
# Tests for the Unix sort utility
# Test Posix features except for locale.
# Test some nonstandard features if present.

AWK=awk
CC=${CC:-${TESTCC:-cc}}
LC_ALL=C
NAWK=nawk
SUM=sum
SORT=sort
TMP=sort.tmp

export TEST AWK CC LC_ALL SUM SORT TMP

case $# in
0)	set $SORT ;;
esac

# Some intermediate file tests require ~100 fds,
# and some systems have low ~64 fds defaults.
(ulimit -n 128) >/dev/null 2>&1 && ulimit -n 128

# Other tests may be needed for files too big to fit in memory;
# see TEST=15 below

SORT=$1
shift
SORT_ARGS="$@"
path=`pwd`:$PATH:/usr/5bin:/bin:/usr/bin
ifs=${IFS-'
	 '}
IFS=":"
set $path
IFS=$ifs
for dir
do	case $dir in
	"")	continue ;;
	esac
	case $NAWK in
	*/*)	;;
	*)	if	test -x $dir/$NAWK
		then	NAWK=$dir/$NAWK
		else	case $AWK in
			*/*)	;;
			*)	test -x $dir/$AWK && AWK=$dir/$AWK ;;
			esac
		fi
		;;
	esac
	case $SORT in
	*/*)	;;
	*)	test -x $dir/$SORT && SORT=$dir/$SORT ;;
	esac
	case $SUM in
	*/*)	;;
	*)	if	test -x $dir/$SUM
		then	AB='abc
pdq
xyz'
			BA='xyz
pdq
abc'
			ab=`echo "$AB" | $SUM 2>/dev/null`
			ba=`echo "$BA" | $SUM 2>/dev/null`
			case $ab in
			$ba)	SUM=$dir/$SUM
				;;
			*)	for old in -o '-o 2' -s -xatt
				do	ab=`echo "$AB" | $dir/$SUM $old 2>/dev/null`
					case $ab in
					?*)	ba=`echo "$BA" | $dir/$SUM $old 2>/dev/null`
						case $ab in
						$ba)	SUM="$dir/$SUM $old"
							break
							;;
						esac
						;;
					esac
				done
				;;
			esac
		fi
		;;
	esac
done
case $NAWK in
*/*)	AWK=$NAWK ;;
esac
echo pathname and options of item under test
echo "	$SORT	$SORT_ARGS"
echo
SORT="$SORT $SORT_ARGS"

# All work done in local temp dir.

trap "cd ..; rm -rf $TMP; exit" 0 1 2 13 15
rm -rf $TMP
if	mkdir $TMP
then	cd $TMP
else	echo "mkdir $TMP FAILED"
	exit 1
fi

# Initialize switches for nonstandard features.
# Use parenthesized settings for supported features.

o=:	# officially obsolescent features: +1 -2
O=:	# really obsolescent features: displaced -o (o=)
g=:	# -g numeric sort including e-format numbers (g=)
k=:	# -Rr -k.p.l fixed records and fields
M=:	# -M sort by month names (M=)
s=:	# -s stable, do not compare raw bytes on equal keys (s=)
S=:	# -S unstable, compare raw bytes on equal keys (S=)
x=:	# -x method, alternate sort method (x=)
X=:	# -Xtest, -Xread ensures read over memory map
y=	# -y user-specified memory size (y=-y4096)

# Detect what features are supported, assuming bad options cause
# errors.  Set switches accordingly.

echo obsolescent and nonstandard features recognized, if any
if $SORT +0 </dev/null 2>/dev/null; then o=
				echo '	+1 -2'; fi
if $SORT /dev/null -o xx 2>/dev/null; then O=
				echo '	displaced -o'; fi
if $SORT -g </dev/null 2>/dev/null; then g=
				echo '	-g g-format numbers'; fi
if test Xcx.by.az = X`echo by.cx.az | $SORT -R3 -k.2.1 2>/dev/null`; then k=
				echo '	-Rr -k.p:l fixed length records'; fi
if $SORT -M </dev/null 2>/dev/null; then M=
				echo '	-M months'; fi
if $SORT -s </dev/null 2>/dev/null; then s=
				echo '	-s stable'; fi
if $SORT -S </dev/null 2>/dev/null; then S=
				echo '	-S unstable'; fi
if $SORT -x default </dev/null 2>/dev/null; then x=
				echo '	-x method'; fi
if test Xok = X`$SORT -Xtest 2>/dev/null`; then X=
				echo '	-Xread ensures read instead of memory map'; fi
if $SORT -y4096 </dev/null 2>/dev/null; then y=-y4096
				echo '	-y space'; fi
if $SORT -z10000 </dev/null 2>/dev/null; then
				echo '	-z recsize (not exercised)'; fi
if $SORT -T. </dev/null 2>/dev/null; then
				echo '	-T tempdir (not exercised)'; fi

# xsort testno options
# Sort file "in" with specified options.
# Compare with file "out" if that is supplied,
# otherwise make plausibility checks on output

cat >xsort <<!

	case \$1 in
	-)	F=-; shift ;;
	*)	F=in ;;
	esac
	X=\$1; shift

	if \$SORT "\$@" \$F >xx  &&  \$SORT -c "\$@" xx 2>/dev/null
	then 
		if test -f out
		then
			cmp -s xx out >/dev/null && exit 0
			echo \$TEST\$X comparison FAILED
		else
			test "\`$SUM <in\`" = "\`$SUM <xx\`" && exit 0
			echo \$TEST\$X checksum FAILED
		fi
	else
		echo \$TEST\$X FAILED
	fi
	exit 1
!
chmod +x xsort

# ysort options
# called to check diagnostics

cat <<! >ysort
error=0 warning=0 works=0
$SORT "\$@" 2>out <in >in1 || error=1
test -s out && warning=1
test -s in1 && works=1
case \$error\$warning\$works in
000)	echo "	\$@" does not indicate trouble, but does not sort ;;
001)	echo "	\$@" does not indicate trouble ;;
010)	echo "	\$@" warns, does not sort, and yields exit status zero ;;
011)	echo "	\$@" warns and continues ;;
100)	echo "	\$@" yields nonzero exit status and does not sort ;;
101)	echo "	\$@" yields nonzero exit status, but sorts ;;
111)	echo "	\$@" warns and yields nonzero exit status, but sorts
esac
!
chmod +x ysort

# linecount testno file count
# declares the given "testno" to be in error if number of
# lines in "file" differs from "count"

cat >linecount <<'!'
$AWK 'END{ if(NR!='$3') print "'$TEST$1' FAILED" }' $2
!
chmod +x linecount

# instability by default for the standard

case $S in
'')	SORT="$SORT -S" ;;
esac

# check behavior in questionable cases

echo
echo "behavior in questionable cases (other than message and exit)"
echo hi >in

rm -f nosuchfile
./ysort nosuchfile

if	test ! -f unwritablefile
then	echo x >unwritablefile
	chmod 0 unwritablefile
fi
./ysort -o unwritablefile </dev/null

cat in | ./ysort - -

./ysort -- in

for i in -k -k0 -k-1 -k1, -kb -k1,2u -t -o
do	./ysort $i
done

$g ./ysort -n -g
$M ./ysort -n -M
./ysort -d -i
./ysort -n -i
./ysort -c in in
./ysort -t. -t:

for i in -a -e -h -j -p -q -v -w -x -0 -1 -2
do	./ysort $i
done

$g false && ./ysort -g
$s false && ./ysort -s
$z false && ./ysort -z

case "$y" in
"")	./ysort -y
esac

./ysort -k1,1 -f

cat >in <<!
A	b
a	bc
a	Bd
B
!
cat >in1 <<!
A	b
B
a	bc
a	Bd
!
cat >in2 <<!
A	b
B
a	Bd
a	bc
!

if $SORT -k1,1 -f -k2,2 <in >out 2>/dev/null
then
	opt="-k1,1 -f -k2,2 :"
	if cmp -s out in >/dev/null
	then	echo "	$opt -f applies to fields 1 and 2"
	elif cmp -s out in1 >/dev/null
	then	echo "	$opt -f applies to field 2 only"
	elif cmp -s out in1 >/dev/null
	then	echo "	$opt -f ineffectual"
	elif cmp -s out /dev/null >/dev/null
	then	echo "	$opt exit status zero, but no output"
	else	echo "	$opt inexplicable"
	fi
fi

# generate large data here to smooth the test feedback

echo
echo "generating large data files for tests that follow (long)"
$AWK 'BEGIN { for(i=0; i<100000; i++) print rand() }' </dev/null |
	 grep -v e >14.in
$AWK 'BEGIN {
	x = "xxxxxxxxxx" 
	x = x x x x x x x x
	for(i=0; i<8000; i++) print rand(), x }' >15.in </dev/null
rm -f in out

echo
echo test numbers denote progress, not trouble

# check for and loop on alternate methods

case $x in
'')	set `$SORT --list < /dev/null | sed 's/[ 	].*//'` ;;
*)	set '' ;;
esac

COMMAND=$SORT
while	:
do	case $# in
	0)		break ;;
	esac
	METHOD=$1
	shift
	case $METHOD in
	check|copy|verify)
			continue
			;;
	'')		echo
			;;
	*)		SORT="$COMMAND -x $METHOD"
			echo
			echo "testing the $METHOD method"
			echo
			;;
	esac

#---------------------------------------------------------------
TEST=01; echo $TEST	"-c status, checksum"
			# obsolescent features go together
cat >in <<!
b
a
!
rm -f out -o

$SORT -c in 2>/dev/null && echo ${TEST}A FAILED

./xsort B || echo ${TEST}B $SUM is probably unsuitable - see comments

$o $SORT -o in +0 in || echo ${TEST}C FAILED

#---------------------------------------------------------------
TEST=02; echo $TEST	"output from -c"
cat >in <<!
x
y
!

$SORT -cr in >out 2>xx && echo ${TEST}A FAILED
test -s out && echo ${TEST}B FAILED
test -s xx || echo option -c is quiet "(legal, not classical)"

$SORT -c /dev/null 2>xx || echo ${TEST}C FAILED
test -s xx && echo ${TEST}D FAILED

#---------------------------------------------------------------
TEST=03; echo $TEST	"-n"
cat >in <<!
-99.0
-99.1
-.0002
  -10
2
2.0x
2.0.1
0010.000000000000000000000000000000000001
10
3x
x
!
cat >out <<!
-99.1
-99.0
  -10
-.0002
x
2
2.0.1
2.0x
3x
10
0010.000000000000000000000000000000000001
!

./xsort "" -n

#---------------------------------------------------------------
TEST=04; echo $TEST	"-b without fields, piping, -c status return"
cat >in <<!
  b
 a
!
cp in out

./xsort A -b

cat in | $SORT | cat >xx
cmp -s xx out >/dev/null || echo ${TEST}B FAILED

$SORT in | $SORT -cr 2>/dev/null && echo ${TEST}C FAILED

#---------------------------------------------------------------
TEST=05; echo $TEST	"fields, reverse fields, -c status return"
cat >in <<!
b b p
a b q
x a
!
cat >out <<!
x a
a b q
b b p
!

$o ./xsort A +1 -2

$o ./xsort B +1 -2 +2r

./xsort C -k 2,2

./xsort D -k 2,2 -k 3r

./xsort E -k 2,2.0

./xsort F -k 2,2 -k 1,1 -k 3

$SORT -c -k 2 in 2>/dev/null && ${TEST}G FAILED

#---------------------------------------------------------------
TEST=06; echo $TEST	"-t"
cat >in <<!
a:
a!
!
cp in out

$o ./xsort A -t : -r +0

$o ./xsort B -t : +0 -1

./xsort C -t : -r -k 1

./xsort D -t : -k 1,1

#---------------------------------------------------------------
TEST=07; echo $TEST	"-t, character positions in fields"
	# -t: as 1 arg is not strictly conforming, but classical
cat >in <<!
: ab
:bac
!
cat >out <<!
:bac
: ab
!

$o ./xsort A -b -t: +1.1
	
$o ./xsort B -t: +1.1r

./xsort C -b -t: -k 2.2

./xsort D -t: -k 2.2r

#---------------------------------------------------------------
TEST=08; echo $TEST	"space and tab as -t characters"
cat >in <<!
 b c
 b	c
	b c
!
cp in out

./xsort A -t ' ' -k2,2

./xsort B -t ' ' -k2.1,2.0

cat >out <<!
 b c
	b c
 b	c
!

./xsort C -t '	' -k2,2

./xsort D -t '	' -k2.1,2.0

cat >out <<!
 b	c
	b c
 b c
!

./xsort E -k2

cat >out <<!
	b c
 b	c
 b c
!

./xsort F -k2b

#---------------------------------------------------------------
TEST=09; echo $TEST	"alphabetic as -t character"
cat >in <<!
zXa
yXa
zXb
!
cp in out

./xsort "" -tX -k2 -k1r,1

#---------------------------------------------------------------
TEST=10; echo $TEST	"-m"
cat >in <<!
a
ab
ab
bc
ca
!
cat >in1 <<!
Z
a
aa
ac
c
!
cat >out <<!
Z
a
a
aa
ab
ab
ac
bc
c
ca
!

$SORT -m in in1 >xx
cmp -s xx out >/dev/null || echo ${TEST}A FAILED

#---------------------------------------------------------------
TEST=11; echo $TEST	"multiple files, -o overwites input, -m, -mu"
cat >in <<!
a
b
c
d
!

$SORT -o xx     in in in in in in in in in in in in in in in in in
./linecount A xx 68
$SORT -o in -mu in in in in in in in in in in in in in in in in in
./linecount B in 4
$SORT -o in -m  in in in in in in in in in in in in in in in in in

cmp -s in xx >/dev/null || echo ${TEST}C FAILED

#---------------------------------------------------------------
TEST=12; echo $TEST	"does -mu pick the first among equals?"
cat >in <<!
3B
3b
3B2
~3B2
4.1
41
5
5.
!
cat >out <<!
3B
3B2
4.1
5
!

./xsort A -mudf || echo "(other behavior is legal, not classical)"

./xsort B -mudf -k1 || echo "(other behavior is legal, not classical)"

#---------------------------------------------------------------
TEST=13; echo $TEST	"long records (>8000 bytes, keys >16000), -r"
$AWK '
BEGIN {	x="x"
	for(i=1; i<=12; i++) x = x x
	for(i=15; i<=25; i++) print x i
}' >in </dev/null
$AWK '
BEGIN {	x="x"
	for(i=1; i<=12; i++) x = x x
	for(i=25; i>=15; i--) print x i
}' >out </dev/null

./xsort A -r

./xsort B -k 1,1r -k 1

#---------------------------------------------------------------
TEST=14; echo $TEST	"-n and -u"
cp 14.in in
rm -f out

./xsort A

./xsort B -n

# next test is unclean: xx is a hidden side-effect of xsort

$AWK '
	$0 < x { print "test '${TEST}C' FAILED"; exit }
	"X" $0 != "X" x { print >"out"; x = $0 }
' xx

./xsort C -n -u

#---------------------------------------------------------------
TEST=15; echo $TEST 	"force intermediate files if possible"
#	option -y4096 ($y) should force a multi-stage internal merge
#	option -Xread ($X) should force read over mmap
case "$y" in
"")	echo ${TEST} inadequate test of large files - revise parameters
esac

cp 15.in in
rm -f out

./xsort A -r $y
cat in | ./xsort - B -r $y
$X ./xsort C -r $y -Xread
$X cat in | $X ./xsort - D -r $y -Xread

rm -f in1
$SORT -r -o in1 in
$AWK '$0 "x" != x { print ; x = $0 "x" }' in1 >out

./xsort E -u -r $y
cat in | ./xsort - F -u -r $y
$X ./xsort G -u -r $y -Xread
$X cat in | $X ./xsort - H -u -r $y -Xread

$SORT -r -u -m -o in1 in1
cmp -s in1 out >/dev/null || echo ${TEST}I FAILED
rm in in1 out

#---------------------------------------------------------------
TEST=16; echo $TEST	"-nr, -nm, file name -"
$AWK 'BEGIN { for(i=-100; i<=100; i+=2) printf "%.10d\n", i }' >in </dev/null

$AWK 'BEGIN { for(i=-99; i<=100; i+=2) print i }' </dev/null |
	 $SORT -nr in - >xx
$AWK '$0+0 != 101-NR { print "'${TEST}A' FAILED"; exit }' xx

$AWK 'BEGIN { for(i=-99; i<=100; i+=2) print i }' </dev/null |
	 $SORT -mn - in >xx
$AWK '$0+0 != -101+NR { print "'${TEST}B' FAILED"; exit }' xx

#---------------------------------------------------------------
TEST=17; echo $TEST	"-d, fields without end, modifier override"
cat >in <<!
a-B
a+b
a b
A+b
a	b
!
cat >out <<!
a	b
a b
A+b
a-B
a+b
!

$o ./xsort A -df +0 +0d 

./xsort B -df -k 1 -k 1d

#---------------------------------------------------------------
TEST=18; echo $TEST	"-u on key only"
cat >in <<!
12	y
13	z
12	x
!
cat >out <<!
12	x
12	y
13	z
!

$o ./xsort A +0 -1

./xsort B -k 1,1

$SORT -u -k 1,1 in >xx
./linecount C xx 2

#---------------------------------------------------------------
TEST=19; echo $TEST	"-i, -d, -f"
cat >xx.c <<!
#include <stdio.h>
static void run(int i, int j){ for( ; i<=j; i++) printf("%.3o %c\n",i,i); }
int main(){	run(0, 011);		/* 012=='\n' */
	run(013, 0377);
	return 0; }
!
$CC -o xx.exe xx.c 
./xx.exe >in
cat >xx.c <<!
#include <stdio.h>
static void run(int i, int j){ for( ; i<=j; i++) printf("%.3o %c\n",i,i); }
int main(){ run(0, 011);
	run(013, ' '-1);
	run(0177, 0377);
	run(' ', 0176);
	return 0; }
!
$CC -o xx.exe xx.c
./xx.exe >out

./xsort A -i -k 2

cat >xx.c <<!
#include <stdio.h>
static void run(int i, int j){ for( ; i<=j; i++) printf("%.3o %c\n",i,i); }
int main(){	run(0, 010);		/* 011=='\t', 012=='\n' */
	run(013, ' '-1);
	run(' '+1, '0'-1);
	run('9'+1, 'A'-1);
	run('Z'+1, 'a'-1);
	run('z'+1, 0377);
	run('\t', '\t');
	run(' ', ' ');
	run('0', '9');
	run('A', 'Z');
	run('a', 'z');
	return 0; }
!
$CC -o xx.exe xx.c
./xx.exe >out

./xsort B -d -k 2

cat >xx.c <<!
#include <stdio.h>
static void run(int i, int j){ for( ; i<=j; i++) printf("%.3o %c\n",i,i); }
int main(){	int i;
	run(0, 011);
	run(013, 'A'-1);
	for(i='A'; i<='Z'; i++) 
		printf("%.3o %c\n%.3o %c\n",i,i,i+040,i+040);
	run('Z'+1, 'a'-1);
	run('z'+1, 0377);
	return 0; }
!
$CC -o xx.exe xx.c
./xx.exe >out
rm -f xx.c xx.o xx.exe

./xsort C -f -k 2

#---------------------------------------------------------------
TEST=20; echo $TEST	"-d, -f, -b applies only to fields"
cat >in <<!
 b
'C
a
!
cp in out

./xsort A -d

./xsort B -f

cat >out <<!
 b
a
'C
!

./xsort C -dfb

#---------------------------------------------------------------
TEST=21; echo $TEST	"behavior of null bytes"
cat >xx.c <<'!'
#include <stdio.h>
int main() { printf("\n%cb\n%ca\n",0,0); return 0; }
!
$CC -o xx.exe xx.c
./xx.exe >in
$SORT -u in >xx
cmp -s in xx >/dev/null && echo ${TEST}A FAILED
test "`wc -c <in`" = "`wc -c <xx`" || echo ${TEST}B FAILED
rm -f xx.c xx.o xx.exe

#---------------------------------------------------------------
TEST=22; echo $TEST	"field limits"
cat >in <<!
a	2
a	1
b	2
b	1
!
cat >out <<!
b	1
b	2
a	1
a	2
!

./xsort "" -r -k1,1 -k2n

#---------------------------------------------------------------
TEST=23; echo $TEST	"empty file, compact -o"

echo hi >xx

$SORT -oxx </dev/null
cmp -s xx /dev/null >/dev/null || echo ${TEST}A FAILED

$SORT -c </dev/null || echo ${TEST}B FAILED

$SORT -cu </dev/null || echo ${TEST}C FAILED

#---------------------------------------------------------------
TEST=24; echo $TEST	"many fields"
cat >in <<!
0:2:3:4:5:6:7:8:9
1:1:3:4:5:6:7:8:9
1:2:2:4:5:6:7:8:9
1:2:3:3:5:6:7:8:9
1:2:3:4:4:6:7:8:9
1:2:3:4:5:5:7:8:9
1:2:3:4:5:6:6:8:9
1:2:3:4:5:6:7:7:9
1:2:3:4:5:6:7:8:8
!
cat >out <<!
1:2:3:4:5:6:7:8:8
1:2:3:4:5:6:7:7:9
1:2:3:4:5:6:6:8:9
1:2:3:4:5:5:7:8:9
1:2:3:4:4:6:7:8:9
1:2:3:3:5:6:7:8:9
1:2:2:4:5:6:7:8:9
1:1:3:4:5:6:7:8:9
0:2:3:4:5:6:7:8:9
!

./xsort "" -t: -k9 -k8 -k7 -k6 -k5 -k4 -k3 -k2 -k1

#---------------------------------------------------------------
TEST=25; echo $TEST	"variously specified alpha fields"
			# numbers give the correct orderings
cat >in <<!
01:04:19:01:16:01:21:01 a
02:03:13:15:13:19:15:02  a
03:02:07:09:07:13:09:03   a
04:01:01:03:01:07:03:04    a
05:08:20:16:17:02:20:05 aa
06:07:14:18:14:20:14:06  aa
07:06:08:10:08:14:08:07   aa
08:05:02:04:02:08:02:08    aa
09:16:22:02:22:04:24:13 b
10:15:16:20:19:22:18:14  b
11:14:10:12:10:16:12:15   b
12:13:04:06:04:10:06:16    b
13:24:24:22:24:06:22:21 bb
14:23:18:24:21:24:16:22  bb
15:22:12:14:12:18:10:23   bb
16:21:06:08:06:12:04:24    bb
17:12:21:21:18:03:19:09 ab
18:11:15:19:15:21:13:10  ab
19:10:09:11:09:15:07:11   ab
20:09:03:05:03:09:01:12    ab
21:20:23:17:23:05:23:17 ba
22:19:17:23:20:23:17:18  ba
23:18:11:13:11:17:11:19   ba
24:17:05:07:05:11:05:20    ba
!
$SORT -k2b -k2 in >xx  
	$SORT -c -t: -k2n xx 2>/dev/null || echo ${TEST}A FAILED
$SORT -k2,2.1b -k2 in >xx  
	$SORT -c -t: -k3n xx 2>/dev/null || echo ${TEST}B FAILED
$SORT -k2.3 -k2 in >xx  
	$SORT -c -t: -k4n xx 2>/dev/null || echo ${TEST}C FAILED
$SORT -k2b,2.3 -k2 in >xx  
	$SORT -c -t: -k5n xx 2>/dev/null || echo ${TEST}D FAILED
$SORT -k2.3,2.1b -k2 in >xx  
	$SORT -c -t: -k6n xx 2>/dev/null || echo ${TEST}E FAILED
$SORT -k2,2.1b -k2r in >xx  
	$SORT -c -t: -k7n xx 2>/dev/null || echo ${TEST}F FAILED
$SORT -b -k2,2 -k2 in >xx  
	$SORT -c -t: -k8n xx 2>/dev/null || echo ${TEST}G FAILED
$SORT -b -k2,2b -k2 in >xx 			# perhaps same as G
	$SORT -c -t: -k3n xx 2>/dev/null || echo ${TEST}H FAILED\
 "(standard is not clear on this)"

#---------------------------------------------------------------
TEST=26; echo $TEST	"empty fields, out of bounds fields	"
cat >in <<!
0 5
1 4
2 3
3 2
4 1
5 0
!
cp in out

./xsort "" -k2.2,2.1 -k2.3,2.4

#---------------------------------------------------------------
TEST=27; echo $TEST	"displaced -o"
rm -f out

$O $SORT /dev/null -o out || $o echo ${TEST}B FAILED
$O test -f out || $O echo ${TEST}C FAILED

#---------------------------------------------------------------
TEST=28; echo $TEST	"apparently nonmonotone field specs"
cat >in <<!
aaaa c
x a
0 b
!
cp in out

$o ./xsort A +1 -0.3 +1.4 -1.5

./xsort B -k2,1.3 -k2.5,2.5

#---------------------------------------------------------------
TEST=29; echo $TEST	"determination of end of option list"
cat >-k <<!
x
!
rm -f out -c

$SORT -- -k </dev/null >xx || echo ${TEST}A argument FAILED
cmp -s xx -k || echo ${TEST}A comparison FAILED

cat >in <<!
xxx
!
>-
>-o
>in1

$SORT -- -o in1 - <in >out
cmp -s in out >/dev/null || echo ${TEST}C FAILED
test -s in1 && echo ${TEST}D FAILED

#---------------------------------------------------------------
TEST=30; echo $TEST	"missing newline"
$AWK 'BEGIN{ printf "%s", "x"}' >in
echo x >out

./xsort "" 2>/dev/null

#---------------------------------------------------------------
TEST=31; echo $TEST	"-M, multiple fields"
cat >in <<!
jan 10 1900
Feb 26 1900
feb 25 1900
January xx 1900
August 11 1900
jan 15 1990
feb 22 1990
mar 15 1990
apr 1 1990
may 45 1990
jun 14 1990
jul 4 1990
aug 1~ 1990
aug 11 1990
sep 1 1990
oct 12 1990
nov 24 1990
dec 25 1990
never 3 1990
 Dec 25 1990
!
cat >out <<!
January xx 1900
jan 10 1900
feb 25 1900
Feb 26 1900
August 11 1900
never 3 1990
jan 15 1990
feb 22 1990
mar 15 1990
apr 1 1990
may 45 1990
jun 14 1990
jul 4 1990
aug 1~ 1990
aug 11 1990
sep 1 1990
oct 12 1990
nov 24 1990
 Dec 25 1990
dec 25 1990
!

$M ./xsort "" -k3n -k1M -k2n

#---------------------------------------------------------------
TEST=32; echo $TEST	"-M case insensitivity, -r"
cat >in <<!
x
june
january
december
!
cat >out <<!
december
june
january
x
!

$M ./xsort "" -Mr

#---------------------------------------------------------------
TEST=33; echo $TEST	"-g, big enough for IEEE floating point"
cat >in <<!
2
1
10
1e-1
.2
1e
1E1
1e.
3e+308
3e307
1e-308
1e-307
!
cat >out <<!
1e-308
1e-307
1e-1
.2
1
1e
1e.
2
10
1E1
3e307
3e+308
!

$g ./xsort "" -g

#---------------------------------------------------------------
TEST=34; echo $TEST	"-g wide operands"
cat >in <<!
.99999999999999999999
099999999999999999999e-21
099999999999999999999e-19
.1e1
!
cat >out <<!
099999999999999999999e-21
.99999999999999999999
.1e1
099999999999999999999e-19
!

$g ./xsort A -g

cat >out <<!
.1e1
.99999999999999999999
099999999999999999999e-19
099999999999999999999e-21
!

./xsort B -n

#---------------------------------------------------------------
TEST=35; echo $TEST	"-g, -u with different fp reps"
cat >in <<!
+0
-0
0
0.10
+.1
-.1
-100e-3x
x
!
cat >out <<!
-.1
-100e-3x
+0
-0
0
x
+.1
0.10
!

$g ./xsort A -g

$g $SORT -gu in >xx && $g $SORT -c -gu xx || echo ${TEST}B FAILED
$g ./linecount C xx 3

#---------------------------------------------------------------
TEST=36; echo $TEST	"-s"
cat >in <<!
a 2
b 1
c 2
a 1
b 2
c 1
!
cat >out <<!
a 2
a 1
b 1
b 2
c 2
c 1
!

$s ./xsort "" -s -k1,1

#---------------------------------------------------------------
TEST=37; echo $TEST	"-s, multiple files"
cat >in <<!
a 2
c 2
!
cat >in1 <<!
a 1
b 1
c 1
!

cat >out <<!
a 2
b 1
c 2
!

$s $SORT -su -k1,1 in in in1 in1 >xx
$s cmp -s xx out >/dev/null || echo ${TEST}A FAILED

cat >out <<!
c 1
b 1
a 1
!

$s $SORT -sru -k1,1 in in in1 in1 >xx
$s cmp -s xx out >/dev/null || echo ${TEST}B FAILED

#---------------------------------------------------------------
TEST=38; echo $TEST	"-s"
$s $AWK '
	BEGIN {
		for(i=1; i<50; i++)
			for(j=1; j<=i; j++) {
				print i, 2 >"in"
				print i, 1 >"in1"
			}
	}' </dev/null

$s $SORT -m -s -k1,1n in in1 >out

$s $AWK '
	func stop()	{ print "'$TEST' FAILED"; exit }
	$1!=last1 	{ if(count!=last1 || $2!=2) stop();
			  count = 0}
	$1==last1 && $2!=last2 { if(count!=last1 || $2!=1) stop();
				 count = 0 }
			{ count++; last1 = $1; last2 = $2 }
	' out

#---------------------------------------------------------------
TEST=39; echo $TEST	"empty fields"
cat >in <<!
bXXa
aXXb
!
cp in out

./xsort A -k3 -tX
./xsort B -k2 -tX
./xsort C -r -k2,2 -tX
./xsort D -r -k4 -tX

#---------------------------------------------------------------
TEST=40; echo $TEST	"deceptive field boundaries"
cat >in <<!
    1.2
  1.10
!
cp in out

./xsort A -t. -k1,1n -k2,2n
./xsort B -t. -k1nr -k2n

cat >in <<!
  feb
  jan
 jan
feb
!
cp in out

$M ./xsort C -k1.1,1.4M
#---------------------------------------------------------------
TEST=41; echo $TEST	"fixed length records"
echo bdpoweonwekjbkgizohoeioilasho > in
echo hozopoonbkkjoieigishwewebdlao > out

$k ./xsort A -s -r -R2 -k.2.1

( echo; echo ) >> in

cat >out <<!
zoweweshpoonoio
lakjhogieibkbd

!

$k ./xsort B -s -r -R2

echo ' aza zaz mzm mam


' > in

echo ' zaz mam mzm aza


' > out

$k ./xsort C -s -r -R4 -k.2.1
$k ./xsort D -s -r -R4 -k1.2,1.2

echo ' zaz mzm mam aza


' > out

$k ./xsort E -s -r -R4 -k.2.2
$k ./xsort F -s -r -R4 -k1.2,1.3
$k ./xsort G -s -r -R4 -k1.2,1.2 -k1.3,1.3

echo ' mzm aza zaz mam


' > out

$k ./xsort H -s -r -R4 -k1.3,1.3 -k1.2,1.2

done
