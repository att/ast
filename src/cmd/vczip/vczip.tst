# regression tests for the vczip command

UNIT vczip

SET pipe-input

function DATA
{
	typeset f

	for f
	do	test -f $f && continue
		case $f in
		hello.dat)
			echo 'hello world' > $f
			;;
		esac
	done
}

TEST 01 builtin vcodex methods

	EXEC	-m rle,huffman
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-q
		SAME INPUT t.rz
		OUTPUT - 'rle^huffman'

TEST 02 vcodex + codex methds

	EXEC	-m rle^huffman^gzip
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rhg
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rhg
		SAME OUTPUT t.dat

	EXEC	-q
		OUTPUT - 'rle^huffman^gzip'

	EXEC	-m rle^huffman^bzip
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rhb
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rhb
		SAME OUTPUT t.dat

	EXEC	-q
		OUTPUT - 'rle^huffman^bzip'

SET nopipe-input

TEST 03 gzip file suffix semantics

	EXEC	-m gzip t.dat
		INPUT t.dat 'this is the data'
		IGNORE OUTPUT

	EXEC	-u t.dat.gz
		INPUT -
		OUTPUT t.dat 'this is the data'

	EXEC	-m bzip u.dat
		INPUT u.dat 'this is more data'
		IGNORE OUTPUT

	EXEC	-u u.dat.bz
		INPUT -
		OUTPUT u.dat 'this is more data'
