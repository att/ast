# tests for the find utility

UMASK 0

function DATA
{
	typeset f i j k
	typeset -Z4 Z
	for f
	do	test -d $f && continue
		KEEP $f
		case $f in
		data)	mkdir data
			for i in aaa zzz
			do	i=data/$i
				mkdir $i
				for j in 111 222 333
				do	mkdir $i/$j
					for k in 4 5 6 7
					do	mkdir $i/$j/$k
						for l in q.c r.d s.z
						do	print $i $j $k $l > $i/$j/$k/$l
							chmod $k$k$k $i/$j/$k/$l
						done
					done
				done
			done
			;;
		empty)	mkdir -p empty
			;;
		match)	mkdir -p match/.ghi match/jkl
			: > match/.abc > match/def
			: > match/.ghi/.mno > match/.ghi/pqr
			: > match/jkl/.stu > match/jkl/vwx
			;;
		mode)	mkdir mode
			for i in 0 1 2 3 4 5 6 7
			do	: > mode/$i$i$i
				chmod $i$i$i mode/$i$i$i
			done
			;;
		not)	mkdir -p not/yet
			: > not/now
			: > not/yet/already
			;;
		size)	mkdir size
			for ((i = 0; i < 600; i += 11))
			do	Z=$i
				for ((j = 0; j < i; j++))
				do	print
				done > size/$Z
			done
			for ((i = 0; i < 6000; i += 321))
			do	Z=$i
				for ((j = 0; j < i; j++))
				do	print
				done > size/$Z
			done
			;;
		time)	mkdir time
			touch -t 'midnight' time/day-0
			touch -t 'midnight 1 day ago' time/day-1
			touch -t 'midnight 2 day ago' time/day-2
			touch -t 'midnight 3 day ago' time/day-3
			;;
		esac
	done
}

TEST 01 'basics'
	DO	DATA data
	EXEC	data -sort -name
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222
data/aaa/222/4
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333
data/aaa/333/4
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222
data/zzz/222/4
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333
data/zzz/333/4
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'
	EXEC	data -sort -name -print
	EXEC	data -sort -name --print
	EXEC	-sort -name -print data
	EXEC	--sort --name --print data
	EXEC	-H data -sort -name --print
	EXEC	--H data -sort -name --print
	EXEC	-metaphysical data -sort -name --print
	EXEC	--metaphysical data -sort -name --print
	EXEC	-L data -sort -name --print
	EXEC	--L data -sort -name --print
	EXEC	-logical data -sort -name --print
	EXEC	--logical data -sort -name --print
	EXEC	-P data -sort -name --print
	EXEC	--P data -sort -name --print
	EXEC	-physical data -sort -name --print
	EXEC	--physical data -sort -name --print
	EXEC	foo
		OUTPUT -
		ERROR - $'find: foo: not found'
		EXIT 1
	EXEC	foo bar
		ERROR - $'find: foo: not found\nfind: bar: not found'

TEST 02 'patterns'
	DO	DATA data
	EXEC	data -sort -name -name '*.c'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/5/q.c
data/aaa/111/6/q.c
data/aaa/111/7/q.c
data/aaa/222/4/q.c
data/aaa/222/5/q.c
data/aaa/222/6/q.c
data/aaa/222/7/q.c
data/aaa/333/4/q.c
data/aaa/333/5/q.c
data/aaa/333/6/q.c
data/aaa/333/7/q.c
data/zzz/111/4/q.c
data/zzz/111/5/q.c
data/zzz/111/6/q.c
data/zzz/111/7/q.c
data/zzz/222/4/q.c
data/zzz/222/5/q.c
data/zzz/222/6/q.c
data/zzz/222/7/q.c
data/zzz/333/4/q.c
data/zzz/333/5/q.c
data/zzz/333/6/q.c
data/zzz/333/7/q.c'
	EXEC	data -sort -name -name '*.c' -print
	EXEC	data -sort -name -name '*.[cd]'
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/7/q.c
data/zzz/333/7/r.d'
	EXEC	data -sort -name \( -name '*.c' \|\| -name '*.d' \)
	EXEC	data -sort -name \( -name '*.c' -or -name '*.d' \)
	EXEC	data -sort -name \( -name '*.c' -or -name '*.d' \) -print
	EXEC	data -sort -name \( -name '*.c' -prune \) -or \( -name '*.d' -print \)
		OUTPUT - $'data/aaa/111/4/r.d
data/aaa/111/5/r.d
data/aaa/111/6/r.d
data/aaa/111/7/r.d
data/aaa/222/4/r.d
data/aaa/222/5/r.d
data/aaa/222/6/r.d
data/aaa/222/7/r.d
data/aaa/333/4/r.d
data/aaa/333/5/r.d
data/aaa/333/6/r.d
data/aaa/333/7/r.d
data/zzz/111/4/r.d
data/zzz/111/5/r.d
data/zzz/111/6/r.d
data/zzz/111/7/r.d
data/zzz/222/4/r.d
data/zzz/222/5/r.d
data/zzz/222/6/r.d
data/zzz/222/7/r.d
data/zzz/333/4/r.d
data/zzz/333/5/r.d
data/zzz/333/6/r.d
data/zzz/333/7/r.d'
	EXEC	data -sort -name \( -name 'zzz|*.c' -prune \) -or \( -name '*.d' -print \)
		OUTPUT - $'data/aaa/111/4/r.d
data/aaa/111/5/r.d
data/aaa/111/6/r.d
data/aaa/111/7/r.d
data/aaa/222/4/r.d
data/aaa/222/5/r.d
data/aaa/222/6/r.d
data/aaa/222/7/r.d
data/aaa/333/4/r.d
data/aaa/333/5/r.d
data/aaa/333/6/r.d
data/aaa/333/7/r.d'

TEST 03 'types'
	DO	DATA data
	EXEC	data -sort -name -type d
		OUTPUT - $'data
data/aaa
data/aaa/111
data/aaa/111/4
data/aaa/111/5
data/aaa/111/6
data/aaa/111/7
data/aaa/222
data/aaa/222/4
data/aaa/222/5
data/aaa/222/6
data/aaa/222/7
data/aaa/333
data/aaa/333/4
data/aaa/333/5
data/aaa/333/6
data/aaa/333/7
data/zzz
data/zzz/111
data/zzz/111/4
data/zzz/111/5
data/zzz/111/6
data/zzz/111/7
data/zzz/222
data/zzz/222/4
data/zzz/222/5
data/zzz/222/6
data/zzz/222/7
data/zzz/333
data/zzz/333/4
data/zzz/333/5
data/zzz/333/6
data/zzz/333/7'
	EXEC	data -sort -name -type f
		OUTPUT - $'data/aaa/111/4/q.c
data/aaa/111/4/r.d
data/aaa/111/4/s.z
data/aaa/111/5/q.c
data/aaa/111/5/r.d
data/aaa/111/5/s.z
data/aaa/111/6/q.c
data/aaa/111/6/r.d
data/aaa/111/6/s.z
data/aaa/111/7/q.c
data/aaa/111/7/r.d
data/aaa/111/7/s.z
data/aaa/222/4/q.c
data/aaa/222/4/r.d
data/aaa/222/4/s.z
data/aaa/222/5/q.c
data/aaa/222/5/r.d
data/aaa/222/5/s.z
data/aaa/222/6/q.c
data/aaa/222/6/r.d
data/aaa/222/6/s.z
data/aaa/222/7/q.c
data/aaa/222/7/r.d
data/aaa/222/7/s.z
data/aaa/333/4/q.c
data/aaa/333/4/r.d
data/aaa/333/4/s.z
data/aaa/333/5/q.c
data/aaa/333/5/r.d
data/aaa/333/5/s.z
data/aaa/333/6/q.c
data/aaa/333/6/r.d
data/aaa/333/6/s.z
data/aaa/333/7/q.c
data/aaa/333/7/r.d
data/aaa/333/7/s.z
data/zzz/111/4/q.c
data/zzz/111/4/r.d
data/zzz/111/4/s.z
data/zzz/111/5/q.c
data/zzz/111/5/r.d
data/zzz/111/5/s.z
data/zzz/111/6/q.c
data/zzz/111/6/r.d
data/zzz/111/6/s.z
data/zzz/111/7/q.c
data/zzz/111/7/r.d
data/zzz/111/7/s.z
data/zzz/222/4/q.c
data/zzz/222/4/r.d
data/zzz/222/4/s.z
data/zzz/222/5/q.c
data/zzz/222/5/r.d
data/zzz/222/5/s.z
data/zzz/222/6/q.c
data/zzz/222/6/r.d
data/zzz/222/6/s.z
data/zzz/222/7/q.c
data/zzz/222/7/r.d
data/zzz/222/7/s.z
data/zzz/333/4/q.c
data/zzz/333/4/r.d
data/zzz/333/4/s.z
data/zzz/333/5/q.c
data/zzz/333/5/r.d
data/zzz/333/5/s.z
data/zzz/333/6/q.c
data/zzz/333/6/r.d
data/zzz/333/6/s.z
data/zzz/333/7/q.c
data/zzz/333/7/r.d
data/zzz/333/7/s.z'

TEST 04 'modes'
	DO	DATA mode
	EXEC	mode -sort -name -perm -000
		OUTPUT - 'mode
mode/000
mode/111
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm  000
		OUTPUT - 'mode/000'
	EXEC	mode -sort -name -perm +000
		OUTPUT -
	EXEC	mode -sort -name -perm -111
		OUTPUT - 'mode
mode/111
mode/333
mode/555
mode/777'
	EXEC	mode -sort -name -perm  111
		OUTPUT - 'mode/111'
	EXEC	mode -sort -name -perm +111
		OUTPUT - 'mode
mode/111
mode/333
mode/555
mode/777'
	EXEC	mode -sort -name -perm -200
		OUTPUT - 'mode
mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -sort -name -perm  200
		OUTPUT -
	EXEC	mode -sort -name -perm +200
		OUTPUT - 'mode
mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -sort -name -perm -220
	EXEC	mode -sort -name -perm  220
		OUTPUT -
	EXEC	mode -sort -name -perm +220
		OUTPUT - 'mode
mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -sort -name -perm -222
	EXEC	mode -sort -name -perm  222
		OUTPUT - 'mode/222'
	EXEC	mode -sort -name -perm +222
		OUTPUT - 'mode
mode/222
mode/333
mode/666
mode/777'
	EXEC	mode -sort -name -perm -333
		OUTPUT - 'mode
mode/333
mode/777'
	EXEC	mode -sort -name -perm  333
		OUTPUT - 'mode/333'
	EXEC	mode -sort -name -perm +333
		OUTPUT - 'mode
mode/111
mode/222
mode/333
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm -444
		OUTPUT - 'mode
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm  444
		OUTPUT - 'mode/444'
	EXEC	mode -sort -name -perm +444
		OUTPUT - 'mode
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm -555
		OUTPUT - 'mode
mode/555
mode/777'
	EXEC	mode -sort -name -perm  555
		OUTPUT - 'mode/555'
	EXEC	mode -sort -name -perm +555
		OUTPUT - 'mode
mode/111
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm -666
		OUTPUT - 'mode
mode/666
mode/777'
	EXEC	mode -sort -name -perm  666
		OUTPUT - 'mode/666'
	EXEC	mode -sort -name -perm +666
		OUTPUT - 'mode
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'
	EXEC	mode -sort -name -perm -777
		OUTPUT - 'mode
mode/777'
	EXEC	mode -sort -name -perm  777
	EXEC	mode -sort -name -perm +777
		OUTPUT - 'mode
mode/111
mode/222
mode/333
mode/444
mode/555
mode/666
mode/777'

TEST 05 '-name pattern'
	DO	DATA match
	EXEC	match -sort -name
		OUTPUT - $'match
match/.abc
match/.ghi
match/.ghi/.mno
match/.ghi/pqr
match/def
match/jkl
match/jkl/.stu
match/jkl/vwx'
	EXEC	match -sort -name -name '*'
	EXEC	match -sort -name -name '.*'
		OUTPUT - $'match/.abc
match/.ghi
match/.ghi/.mno
match/jkl/.stu'

TEST 06 '-size number[bckw]'
	DO	DATA size
	EXEC	size -sort -name -type f
		OUTPUT - $'size/0000
size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -0
		OUTPUT -
	EXEC	size -sort -name -type f -a -size 0
		OUTPUT - $'size/0000'
	EXEC	size -sort -name -type f -a -size +0
		OUTPUT - $'size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -1c
		OUTPUT - $'size/0000'
	EXEC	size -sort -name -type f -a -size 1c
		OUTPUT -
	EXEC	size -sort -name -type f -a -size +1c
		OUTPUT - $'size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -176c
		OUTPUT - $'size/0000
size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165'
	EXEC	size -sort -name -type f -a -size 176c
		OUTPUT - $'size/0176'
	EXEC	size -sort -name -type f -a -size +176c
		OUTPUT - $'size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -1w
		OUTPUT - $'size/0000'
	EXEC	size -sort -name -type f -a -size 1w
		OUTPUT -
	EXEC	size -sort -name -type f -a -size +1w
		OUTPUT - $'size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -1b
		OUTPUT - $'size/0000'
	EXEC	size -sort -name -type f -a -size 1b
		OUTPUT - $'size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506'
	EXEC	size -sort -name -type f -a -size +1b
		OUTPUT - $'size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963
size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'
	EXEC	size -sort -name -type f -a -size -1k
		OUTPUT - $'size/0000'
	EXEC	size -sort -name -type f -a -size 1k
		OUTPUT - $'size/0011
size/0022
size/0033
size/0044
size/0055
size/0066
size/0077
size/0088
size/0099
size/0110
size/0121
size/0132
size/0143
size/0154
size/0165
size/0176
size/0187
size/0198
size/0209
size/0220
size/0231
size/0242
size/0253
size/0264
size/0275
size/0286
size/0297
size/0308
size/0319
size/0321
size/0330
size/0341
size/0352
size/0363
size/0374
size/0385
size/0396
size/0407
size/0418
size/0429
size/0440
size/0451
size/0462
size/0473
size/0484
size/0495
size/0506
size/0517
size/0528
size/0539
size/0550
size/0561
size/0572
size/0583
size/0594
size/0642
size/0963'
	EXEC	size -sort -name -type f -a -size +1k
		OUTPUT - $'size/1284
size/1605
size/1926
size/2247
size/2568
size/2889
size/3210
size/3531
size/3852
size/4173
size/4494
size/4815
size/5136
size/5457
size/5778'

TEST 07 '[ -exec -xargs ] X {} X [ ; + ]'
	DO	DATA empty mode
	EXEC	empty -exec echo \;
		OUTPUT - $'empty'
	EXEC	empty -exec echo {} \;
	EXEC	empty -exec echo {} {} \;
		OUTPUT - $'empty empty'
	EXEC	empty -exec echo {} {} {} \;
		OUTPUT - $'empty empty empty'
	EXEC	mode -sort -name -exec echo ';'
		OUTPUT - $'mode\nmode/000\nmode/111\nmode/222\nmode/333\nmode/444\nmode/555\nmode/666\nmode/777'
	EXEC	mode -sort -name -exec echo {} ';'
	EXEC	mode -sort -name -xargs echo ';'
		OUTPUT - $'mode mode/000 mode/111 mode/222 mode/333 mode/444 mode/555 mode/666 mode/777'
	EXEC	mode -sort -name -xargs echo {} ';'
	EXEC	mode -sort -name -xargs echo {} '+'
	EXEC	mode -sort -name -exec echo {} '+'
	EXEC	mode -sort -name -exec echo '+'
		OUTPUT -
		ERROR - $'find: incomplete statement'
		EXIT 1
	EXEC	mode -sort -name -xargs echo '+'

TEST 08 '-mtime [-+]N'
	DO	DATA time
	EXEC	time -type f -mtime 0
		OUTPUT - $'time/day-0'
	EXEC	time -type f -mtime 1
		OUTPUT - $'time/day-1'
	EXEC	time -type f -mtime 2
		OUTPUT - $'time/day-2'
	EXEC	time -type f -mtime 3
		OUTPUT - $'time/day-3'
	EXEC	time -type f -mtime 4
		OUTPUT -
	EXEC	time -sort name -type f -mtime -0
	EXEC	time -sort name -type f -mtime -1
		OUTPUT - $'time/day-0'
	EXEC	time -sort name -type f -mtime -2
		OUTPUT - $'time/day-0\ntime/day-1'
	EXEC	time -sort name -type f -mtime -3
		OUTPUT - $'time/day-0\ntime/day-1\ntime/day-2'
	EXEC	time -sort name -type f -mtime -4
		OUTPUT - $'time/day-0\ntime/day-1\ntime/day-2\ntime/day-3'
	EXEC	time -sort name -type f -mtime +0
		OUTPUT - $'time/day-1\ntime/day-2\ntime/day-3'
	EXEC	time -sort name -type f -mtime +1
		OUTPUT - $'time/day-2\ntime/day-3'
	EXEC	time -sort name -type f -mtime +2
		OUTPUT - $'time/day-3'
	EXEC	time -sort name -type f -mtime +3
		OUTPUT -
	EXEC	time -sort name -type f -mtime +4

TEST 09 '! implicit -print bug'
	DO	DATA not
	EXEC	not ! -type f
		OUTPUT - $'not\nnot/yet'
	EXEC	not ! -type f -print
	EXEC	not '(' ! -type f ')' -print
