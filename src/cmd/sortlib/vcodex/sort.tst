# regression tests for the sort vcodex discipline library

TITLE + vcodex

export LC_ALL=C

KEEP "*.dat" "*.qz"

function DATA
{
	typeset -Z5 i
	for f
	do	test -d $f.dat && continue
		KEEP $f.dat $f.qz
		case $f in
		rev)	n=9999
			for ((i = 0; i <= n; i++))
			do	print $i
			done > $f.sorted
			for ((i = n; i >= 0; i--))
			do	print $i
			done > $f.dat
			;;
		esac
		vczip -mq < $f.dat > $f.qz
	done
}

TEST 01 'basics'

	DO	DATA rev

	EXEC -o rev.out rev.dat
		SAME rev.out rev.sorted

	EXEC -lvcodex,notemporary,nooutput -o rev.out rev.qz
		SAME rev.out rev.sorted

	EXEC -lvcodex,nooutput -o rev.out rev.qz
		SAME rev.out rev.sorted

	EXEC -lvcodex -o rev.tmp rev.qz

	EXEC -lvcodex,notemporary,nooutput -o rev.out rev.tmp.qz
		SAME rev.out rev.sorted

	EXEC -v -za16ki -zb16ki -zi16ki -lvcodex,nooutput,verbose,regress -o rev.out rev.qz
		SAME rev.out rev.sorted
		ERROR - $'sort d record format
sort vcodex decode rev.qz (transpose,rle,huffman)
sort process 16384 -> 16380
sort vcodex encode temporary-2 (transpose,rle,huffman)
sort write intermediate
sort vcodex decode temporary-3
sort process 8196 -> 8196
sort vcodex encode temporary-4 (transpose,rle,huffman)
sort write intermediate
sort vcodex decode temporary-5
sort process 16384 -> 16380
sort vcodex encode temporary-6 (transpose,rle,huffman)
sort write intermediate
sort vcodex decode temporary-7
sort process 8196 -> 8196
sort vcodex encode temporary-8 (transpose,rle,huffman)
sort write intermediate
sort vcodex decode temporary-9
sort process 10848 -> 10848
sort vcodex encode temporary-10 (transpose,rle,huffman)
sort write intermediate
sort vcodex decode temporary-11
sort merge text'

	EXEC -v -za16ki -zb16ki -zi16ki -lvcodex,nooutput,verbose,regress,temporary=rle^huffman -o rev.out rev.qz
		SAME rev.out rev.sorted
		ERROR - $'sort d record format
sort vcodex decode rev.qz (transpose,rle,huffman)
sort process 16384 -> 16380
sort vcodex encode temporary-2 (rle^huffman)
sort write intermediate
sort vcodex decode temporary-3
sort process 8196 -> 8196
sort vcodex encode temporary-4 (rle^huffman)
sort write intermediate
sort vcodex decode temporary-5
sort process 16384 -> 16380
sort vcodex encode temporary-6 (rle^huffman)
sort write intermediate
sort vcodex decode temporary-7
sort process 8196 -> 8196
sort vcodex encode temporary-8 (rle^huffman)
sort write intermediate
sort vcodex decode temporary-9
sort process 10848 -> 10848
sort vcodex encode temporary-10 (rle^huffman)
sort write intermediate
sort vcodex decode temporary-11
sort merge text'
