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
: pax tests

# testpax [ + ] [ - ] [ tar-path ] [ pax-path ] [ pax-options ... ]
#
#	+		execution trace
#	-		don't clean up pax.tmp
#	.		exit on first failure (and don't clean up)
#	tar-path	tar the old and new archives
#
# @(#)testpax (AT&T Labs Research) 2003-10-01

integer errors=0 tests=0
typeset -Z TEST=00

bail=
clobber=1
tmp=pax.tmp

while	:
do	case $1 in
	+)	PS4='+$LINENO+ '; shift; set -x ;;
	-)	clobber=; shift ;;
	.)	bail=1; shift ;;
	*)	break ;;
	esac
done
case $1 in
*tar)	tar=$1; shift; formats="tar.tar" ;;
*)	tar=''; formats="pax.pax pax:gzip.pgz cpio.cpio tar.tar tar:gzip.tgz" ;;
esac
case $1 in
-*|'')	set pax "$@" ;;
esac
set "$@" --physical --nosummary --noverify
case $1 in
/*)	;;
*)	cmd=$1
	shift
	set "$PWD/$cmd" "$@"
	;;
esac
print -u2 "TEST	$@"

for format in $formats
do	suf=${format##*.}
	fmt="--format=${format%.*}"

	: all files created in $tmp

	rm -rf $tmp
	mkdir $tmp
	cd $tmp

	((tests++))
	status=0

	: old files and base archive

	mkdir old
	cd old
	for i in a b c d e f g
	do	print $i$i$i > $i
	done
	ln a i
	ln b j
	ln -s c k
	ln -s d m
	mkdir dirsub
	print aha > dirsub/x
	case $tar in
	'')	"$@" -wf ../old.$suf $fmt * || status=1 ;;
	*)	$tar cf ../old.$suf . || status=1 ;;
	esac
	cd ..

	: new files and base archive

	mkdir new
	cd new
	if	false
	then	"$@" -rf ../old.$suf || status=1
	else	: directory link order matters!
		for i in a b c d e f g
		do	print $i$i$i > $i
		done
		ln a i
		ln b j
		ln -s c k
		ln -s d m
		print "xxxx" > ca
		: have a dir alphabetically between file1 and file2
		mkdir csubdir	
		print "ccc" > csubdir/h
		mkdir dirsub
		: have file1 alphabetically less than file2
		ln ca dirsub/y	
		print aha > dirsub/x
	fi
	rm a
	sleep 1
	print DDD > d
	print zzz > z
	rm m
	ln -s e m
	mkdir beta
	print ppp > beta/p
	print qqq > beta/q
	print rrr > beta/r
	case $tar in
	'')	"$@" -wf ../new.$suf $fmt * || status=1 ;;
	*)	$tar cf ../new.$suf . || status=1 ;;
	esac
	cd ..

	: delta archive

	"$@" -rf new.$suf -wf old-new.$suf $fmt -z old.$suf || status=1

	: replicate new archive

	"$@" -rf old-new.$suf -z old.$suf -wf rep.$suf $fmt || status=1

	: verify replica

	mkdir rep
	cd rep
	"$@" -rf ../rep.$suf || status=1
	cd ..
	diff -r new rep
	case $status$? in
	00)	result= ;;
	*)	result=" FAILED"; ((errors++)) ;;
	esac
	TEST=$tests
	print -u2 "$TEST	base $fmt$result"
	case $bail:$result in
	1:?*)	exit 1 ;;
	esac

	((tests++))
	status=0

	: add subdir to old

	sleep 1
	mkdir old/asub
	echo "zzz" > old/asub/w

	: add same subdir to new but with newer time

	sleep 1
	mkdir new/asub
	echo "zzz" > new/asub/w

	: redo the old and new base archives

	cd old
	case $tar in
	'')	"$@" -wf ../old.$suf $fmt * || status=1 ;;
	*)	$tar cf ../old.$suf . || status=1 ;;
	esac
	cd ../new
	case $tar in
	'')	"$@" -wf ../new.$suf $fmt * || status=1 ;;
	*)	$tar cf ../new.$suf . || status=1 ;;
	esac
	cd ..

	: delta archive

	"$@" -rf new.$suf -wf old-new.$suf $fmt -z old.$suf || status=1

	: replicate new archive

	"$@" -rf old-new.$suf -z old.$suf -wf rep.$suf $fmt || status=1

	: verify replica

	rm -rf rep
	mkdir rep
	cd rep
	"$@" -rf ../rep.$suf || status=1
	cd ..
	diff -r new rep
	case $status$? in
	00)	result= ;;
	*)	result=" FAILED"; ((errors++)) ;;
	esac
	TEST=$tests
	print -u2 "$TEST	delta $fmt$result"
	case $bail:$result in
	1:?*)	exit 1 ;;
	esac

	((tests++))
	status=0

	: verify dir to ar to dir to ar

	cd old
	"$@" -wf ../o . || status=1
	cd ../new
	rm -r *
	"$@" -rf ../o || status=1
	"$@" -wf ../n . || status=1
	cd ..
	"$@" -vf o 2>/dev/null | sed -e '/^l/s/ ... [ 0-9][0-9] [0-9][0-9]:[0-9][0-9] / /' > oo || status=1
	"$@" -vf n 2>/dev/null | sed -e '/^l/s/ ... [ 0-9][0-9] [0-9][0-9]:[0-9][0-9] / /' > nn || status=1
	diff oo nn
	case $status$? in
	00)	result= ;;
	*)	result=" FAILED"; ((errors++)) ;;
	esac
	TEST=$tests
	print -u2 "$TEST	io $fmt$result"
	case $bail:$result in
	1:?*)	exit 1 ;;
	esac

	: back to parent of $tmp
	cd ..
done
cd $tmp

: rw test

((tests++))
status=0
mkdir cpy
"$@" -rw new cpy || status=1
diff -r new cpy/new
case $status$? in
00)	result= ;;
*)	result=" FAILED"; ((errors++)) ;;
esac
TEST=$tests
print -u2 "$TEST	copy new cpy$result"
case $bail:$result in
1:?*)	exit 1 ;;
esac

: conversion test

((tests++))
x=ascii
echo $x$x$x$x$x$x$x$x$x$x$x$x$x$x$x$x > a
for i in 1 2 3 4
do	cat a a > b
	cat b b > a
done
"$@" -wf a.pax -s ,a,b, --to=ebcdic3 a
"$@" -rf a.pax --from=ebcdic3
if	cmp -s a b
then	result=
else	result=" FAILED"
	((errors++))
fi
TEST=$tests
print -u2 "$TEST	codeset conversion$result"
case $bail:$result in
1:?*)	exit 1 ;;
esac

: clean up

case $clobber in
?*)	cd ..
	rm -rf $tmp
	;;
esac

case $errors in
1)	plural= ;;
*)	plural=s ;;
esac
print -u2 "TEST	$1, $tests tests, $errors error$plural"
