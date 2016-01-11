# regression tests for pzip and pin

KEEP "*.[dp][ar]t"

function DATA
{
	typeset f
	integer i j
	for f
	do	test -f $f && continue
		case $f in
		tst.dat)for ((i = 0; i < 100; i++))
			do	for ((j = 0; j < 100; j++))
				do	print -f $'%02u-%02u\n' $i $j
				done
			done
			;;
		col.prt)print $'6\n0\n1\n2\n3\n4\n5'
			;;
		row.prt)print $'6\n0-5'
			;;
		tst.prt)print $'6\n4'
			;;
		esac > $f
	done
}

TEST 01 'pin'
	DO	DATA tst.dat
	PROG pin -v tst.dat
		OUTPUT - $'# pzip partition
# pin (AT&T Research) 2003-07-17
# row 6 window 60000 compression level 6

6	# high frequency 1

4'
		ERROR - $'row size 6
filter top 10% high frequency columns
filter done -- 10000 rows
1 high frequency column out of 6
reorder pairs for 0 [4]
reorder part 3
reorder done
dynamic 0..0
dynamic done'

TEST 02 'pzip'
	DO	DATA tst.dat col.prt row.prt tst.prt
	EXEC --regress --summary -p col.prt tst.dat
		MOVE OUTPUT tst.pz
		ERROR - $'total tst.dat rate 137.30 time 1.00s bpr 0.04 bps 58.5k size 60000/437 windows 1 records 10000'
	EXEC tst.pz
		SAME OUTPUT tst.dat
		ERROR -
	EXEC --regress --summary -p row.prt tst.dat
		MOVE OUTPUT tst.pz
		ERROR - $'total tst.dat rate 2.88 time 1.00s bpr 2.08 bps 58.5k size 60000/20842 windows 1 records 10000'
	EXEC tst.pz
		SAME OUTPUT tst.dat
		ERROR -
	EXEC --regress --summary -p tst.prt tst.dat
		MOVE OUTPUT tst.pz
		ERROR - $'total tst.dat rate 232.56 time 1.00s bpr 0.03 bps 58.5k size 60000/258 windows 1 records 10000'
	EXEC tst.pz
		SAME OUTPUT tst.dat
		ERROR -
	EXEC --regress --summary -p tst.prt -w 100K tst.dat
		MOVE OUTPUT tst.pz
		ERROR - $'total tst.dat rate 232.56 time 1.00s bpr 0.03 bps 58.5k size 60000/258 windows 1 records 10000'
	EXEC tst.pz
		SAME OUTPUT tst.dat
		ERROR -
