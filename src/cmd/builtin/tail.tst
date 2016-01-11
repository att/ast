# regression tests for the tail utilitiy

KEEP "*.dat"

TWD

export LC_ALL=C

function DATA
{
	typeset f
	integer n
	for f
	do	test -f $f && continue
		case $f in
		10.dat) for ((i = 1; i <= 10; i++))
			do	print $i
			done
			;;
		100.dat)for ((i = 1; i <= 100; i++))
			do	print $i
			done
			;;
		500000.dat)
			for ((i = 1; i <= 500000; i++))
			do	print $i
			done
			;;
		a.dat)	print a
			;;
		b.dat)	print b
			;;
		i.dat)	print -n $'foo\nbar'
			;;
		esac > $f
	done
}

TEST 01 'explicit file operands'
	DO	DATA 10.dat 100.dat
	EXEC	10.dat
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	+0 10.dat
	EXEC	-n +0 10.dat
	EXEC	+1 10.dat
	EXEC	-n +1 10.dat
	EXEC	+2 10.dat
		OUTPUT - $'2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-n +2 10.dat
	EXEC	-9 10.dat
	EXEC	-n -9 10.dat
	EXEC	-1 10.dat
		OUTPUT - $'10'
	EXEC	-n -1 10.dat
	EXEC	+10 10.dat
	EXEC	-n +10 10.dat
	EXEC	100.dat
		OUTPUT - $'91\n92\n93\n94\n95\n96\n97\n98\n99\n100'
	EXEC	-1 100.dat
		OUTPUT - $'100'
	EXEC	-n -1 100.dat
	EXEC	+100 100.dat
	EXEC	-n +100 100.dat
	EXEC	-2 100.dat
		OUTPUT - $'99\n100'
	EXEC	-n -2 100.dat
	EXEC	+99 100.dat
	EXEC	-n +99 100.dat
	EXEC	-c 6 100.dat
		OUTPUT - $'9\n100'
	EXEC	-6c 100.dat
	EXEC	-c +287 100.dat
	EXEC	+287c 100.dat

TEST 02 'headers'
	DO	DATA a.dat b.dat
	EXEC	a.dat
		OUTPUT - $'a'
	EXEC	-v a.dat
		OUTPUT - $'==> a.dat <==\na'
	EXEC	a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
	EXEC	-v a.dat b.dat
	EXEC	-h a.dat
		OUTPUT - $'a'
	EXEC	-h a.dat b.dat
		OUTPUT - $'a\nb'

TEST 03 'timeouts'
	DO	DATA a.dat b.dat
	EXEC	-f -t 2 a.dat
		OUTPUT - $'a'
		ERROR - $'tail: warning: a.dat: 2.00s timeout'
	EXEC	-f -t 2 a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
		ERROR - $'tail: warning: a.dat: 2.00s timeout\ntail: warning: b.dat: 2.00s timeout'
	EXEC	-s -f -t 2 a.dat
		OUTPUT - $'a'
		ERROR -
	EXEC	-s -f -t 2 a.dat b.dat
		OUTPUT - $'==> a.dat <==\na\n\n==> b.dat <==\nb'
	EXEC	-h -s -f -t 2 a.dat
		OUTPUT - $'a'
	EXEC	-h -s -f -t 2 a.dat b.dat
		OUTPUT - $'a\nb'
	EXEC	-h -f -t 2 a.dat
		OUTPUT - $'a'
		ERROR - $'tail: warning: a.dat: 2.00s timeout'
	EXEC	-h -f -t 2 a.dat b.dat
		OUTPUT - $'a\nb'
		ERROR - $'tail: warning: a.dat: 2.00s timeout\ntail: warning: b.dat: 2.00s timeout'

SET pipe-input

TEST 04 'standard input'
	DO	DATA 10.dat 100.dat
	EXEC
		SAME INPUT 10.dat
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-
	EXEC	+0
	EXEC	-n +0
	EXEC	+1
	EXEC	-n +1
	EXEC	+2
		OUTPUT - $'2\n3\n4\n5\n6\n7\n8\n9\n10'
	EXEC	-n +2
	EXEC	-9
	EXEC	-n -9
	EXEC	-1
		OUTPUT - $'10'
	EXEC	-n -1
	EXEC	+10
	EXEC	-n +10
	EXEC	--
		SAME INPUT 100.dat
		OUTPUT - $'91\n92\n93\n94\n95\n96\n97\n98\n99\n100'
	EXEC	-
	EXEC	-1
		OUTPUT - $'100'
	EXEC	-n -1
	EXEC	+100
	EXEC	-n +100
	EXEC	-2
		OUTPUT - $'99\n100'
	EXEC	-n -2
	EXEC	+99
	EXEC	-n +99
	EXEC	-c 6
		OUTPUT - $'9\n100'
	EXEC	-6c
	EXEC	-c +287
	EXEC	+287c

TEST 05 'r combinations'

	EXEC	-r tst.dat
		INPUT tst.dat $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3\n2\n1'

	EXEC	-1r tst.dat
		OUTPUT - $'10'

	EXEC	-2r tst.dat
		OUTPUT - $'10\n9'

	EXEC	-3r tst.dat
		OUTPUT - $'10\n9\n8'

	EXEC	-4r tst.dat
		OUTPUT - $'10\n9\n8\n7'

	EXEC	-5r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6'

	EXEC	-6r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5'

	EXEC	-7r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4'

	EXEC	-8r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3'

	EXEC	-9r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3\n2'

	EXEC	-10r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3\n2\n1'

	EXEC	-11r tst.dat

	EXEC	+r tst.dat

	EXEC	+1r tst.dat

	EXEC	+2r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3\n2'

	EXEC	+3r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4\n3'

	EXEC	+4r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5\n4'

	EXEC	+5r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6\n5'

	EXEC	+6r tst.dat
		OUTPUT - $'10\n9\n8\n7\n6'

	EXEC	+7r tst.dat
		OUTPUT - $'10\n9\n8\n7'

	EXEC	+8r tst.dat
		OUTPUT - $'10\n9\n8'

	EXEC	+9r tst.dat
		OUTPUT - $'10\n9'

	EXEC	+10r tst.dat
		OUTPUT - $'10'

	EXEC	+11r tst.dat
		OUTPUT -

# the remainder converted from the gnu tail Test.pm

TEST 10	chars

	EXEC	+2c
		INPUT -n - $'abcd'
		OUTPUT -n - $'bcd'

TEST 11	obs-c

	EXEC	+8c
		INPUT -n - $'abcd'
		OUTPUT - 

	EXEC	-1c
		INPUT -n - $'abcd'
		OUTPUT -n - $'d'

	EXEC	-9c
		INPUT -n - $'abcd'
		OUTPUT -n - $'abcd'

	EXEC	-12c
		INPUT -n - $'xyyyyyyyyyyyyz'
		OUTPUT -n - $'yyyyyyyyyyyz'

	EXEC	-1l
		INPUT - $'x'
		OUTPUT - $'x'

TEST 12	obs-l

	EXEC	-1l
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+1l
		INPUT - $'x\ny'
		OUTPUT - $'x\ny'

	EXEC	+2l
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	-1
		INPUT - $'x'
		OUTPUT - $'x'

TEST 13	obs

	EXEC	-1
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+1
		INPUT - $'x\ny'
		OUTPUT - $'x\ny'

	EXEC	+2
		INPUT - $'x\ny'
		OUTPUT - $'y'

	EXEC	+c
		INPUT - $'xyyyyyyyyyyz'
		OUTPUT - $'yyz'

	EXEC	-c
		OUTPUT - $'yyyyyyyyz'

	EXEC	+c
		INPUT -n - $'xyyyyyyyyyyz'
		OUTPUT -n - $'yyz'

	EXEC	-c
		OUTPUT -n - $'yyyyyyyyyz'

TEST 14	obsx

	EXEC	+l
		INPUT -n - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT -n - $'y\ny\nz'

	EXEC	+cl

	EXEC	-l
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-cl

TEST 15	empty

	EXEC	-

	EXEC	-c

TEST 16 err

	EXEC	+2cz
		ERROR - $'tail: z: invalid suffix
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'
	    	EXIT 2

	EXEC	-2cX
		ERROR - $'tail: X: invalid suffix
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'

	EXEC	-c99999999999999999999
		ERROR - $'tail: -c: 99999999999999999999: invalid numeric argument -- out of range
Usage: tail [-bfhlLqrsv] [-n lines] [-c[chars]] [-t timeout] [file ...]'

TEST 17	minus

	EXEC	-
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-n 10

TEST 18	n

	EXEC	-n -10
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\ny\ny\ny\ny\ny\ny\ny\nz'

	EXEC	-n +10
		INPUT - $'x\ny\ny\ny\ny\ny\ny\ny\ny\ny\ny\nz'
		OUTPUT - $'y\ny\nz'

	EXEC	-n +0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y\ny\ny\ny\ny'

	EXEC	-n +1
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y\ny\ny\ny\ny'

	EXEC	-n -0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - 

	EXEC	-n -1
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - $'y'

	EXEC	-n  0
		INPUT - $'y\ny\ny\ny\ny'
		OUTPUT - 

TEST 19 VSC#4,5,14,15,1001,1003

	EXEC	- more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-10 more

	EXEC	-l more

	EXEC	-10l more

	EXEC	+ more
		OUTPUT - $'aaa\nbbb\nccc'

	EXEC	+10 more

	EXEC	+l more

	EXEC	+10l more

	EXEC	- less
		INPUT less $'111\n222\n333\n444\n555'
		OUTPUT - $'111\n222\n333\n444\n555'

	EXEC	-l less

	EXEC	+c less
		OUTPUT - $'33\n444\n555'

	EXEC	+c mini
		INPUT mini $'111\n222'
		OUTPUT -

	EXEC	+512c big
		INPUT big $'1bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
2bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
3bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
4bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
5bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
6bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
7bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
8bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh'
		OUTPUT - $'8bcdefgh'

	EXEC	+1b big

	EXEC	-512c big
		OUTPUT - $'2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
2bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
3bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
4bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
5bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
6bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
7bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh
8bcdefgh2bcdefgh3bcdefgh4bcdefgh5bcdefgh6bcdefgh7bcdefgh8bcdefgh'

	EXEC	-1b big

TEST 20 VSC#20

	EXEC	-t1s -s -f -c -7 more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'bb\nccc'

	EXEC	-c -7 more #21

	EXEC	-t1s -s -f -n +10 more
		OUTPUT - $'aaa\nbbb\nccc'

	EXEC	-n +10 more

	EXEC	-t1s -s -f -n -4 more
		OUTPUT - $'999\naaa\nbbb\nccc'

	EXEC	-n -4 more

	EXEC	-t1s -s -f -n +4 more
		OUTPUT - $'444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-n +4 more

	EXEC	-t1s -s -7lf more
		OUTPUT - $'666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-7l more

	EXEC	-t1s -s -7cf more
		OUTPUT - $'bb\nccc'

	EXEC	-7c more

	EXEC	-t1s -s +33cf more
		OUTPUT - $'999\naaa\nbbb\nccc'

	EXEC	+33c more

	EXEC	-t1s -s +12lf more
		OUTPUT - $'ccc'

	EXEC	+12l more

TEST 21 xpg4

	EXEC	-t1s -s -f1 more
		INPUT more $'111\n222\n333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'
		OUTPUT - $'ccc'

	EXEC	-t1s -s -f10 more
		OUTPUT - $'333\n444\n555\n666\n777\n888\n999\naaa\nbbb\nccc'

	EXEC	-t1s -s -f more

SET nopipe-input

TEST 30 'fifo by redirection'

	JOB -f
		FIFO INPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20'
		OUTPUT - $'11\n12\n13\n14\n15\n16\n17\n18\n19\n20'
	EXITED

TEST 31 'fifo by path'

	JOB -f fifo
		FIFO INPUT fifo $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20'
		OUTPUT - $'11\n12\n13\n14\n15\n16\n17\n18\n19\n20'
	CONTINUE
		INPUT fifo $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
		OUTPUT - $'1\n2\n3\n4\n5\n6\n7\n8\n9\n10'
	CONTINUE

function partial
{
	integer n=${1:-4} s=1 i j
	typeset -A f

	for ((i = 1; i <= n; i++))
	do	f[$i]=1
		{
			print $i
			for ((j = 1; j <= i; j++))
			do	print -n "$i $j ... "
				sleep $s
				print $i $j ok
			done 
		} > $i &
	done
}

TEST 32 'tail -f with partial lines'

	PROG partial 4

	EXEC -t2 -f 1 2 3 4
		OUTPUT -e 'egrep -v "^$|^==" | sort' - $'1
1 1 ... 1 1 ok
2
2 1 ... 2 1 ok
2 2 ... 2 2 ok
3
3 1 ... 3 1 ok
3 2 ... 3 2 ok
3 3 ... 3 3 ok
4
4 1 ... 4 1 ok
4 2 ... 4 2 ok
4 3 ... 4 3 ok
4 4 ... 4 4 ok'
		ERROR - $'tail: warning: 1: 2.00s timeout
tail: warning: 2: 2.00s timeout
tail: warning: 3: 2.00s timeout
tail: warning: 4: 2.00s timeout'

	PROG wait
		OUTPUT -
		ERROR -

TEST 33 "-f large initial context"

	DO	DATA 500000.dat

	EXEC	-n 600000 -f -t 1 500000.dat
		SAME OUTPUT 500000.dat
		ERROR - 'tail: warning: 500000.dat: 1.00s timeout'

TEST 34 "no initial context"

	DO	DATA 10.dat

	EXEC	-n 0 10.dat
		OUTPUT -

	EXEC	-n -0 10.dat
		OUTPUT -

	EXEC	-0 10.dat
		OUTPUT -

	EXEC	-n 0 -f -t 1 10.dat
		ERROR - 'tail: warning: 10.dat: 1.00s timeout'

	EXEC	-n -0 -f -t 1 10.dat

	EXEC	-0 -f -t 1 10.dat

	EXEC	-0f -t 1 10.dat

TEST 35 "incomplete line"

	DO	DATA i.dat

	EXEC -n -0 i.dat
		OUTPUT -

	EXEC -n +3 i.dat

	EXEC -n -1 i.dat
		OUTPUT -n - bar

	EXEC -n +2 i.dat

	EXEC -n -2 i.dat
		OUTPUT -n - $'foo\nbar'

	EXEC -n -3 i.dat

	EXEC -n +0 i.dat

	EXEC -n +1 i.dat
