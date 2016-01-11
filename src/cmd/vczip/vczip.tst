# regression tests for the vczip command

UNIT vczip

SET pipe-input

TEST 01 builtin methods

	EXEC	-m rle.0,huffman
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-m rle.0,huffman
		SAME INPUT t.dat
		COPY OUTPUT t.rz

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-m rle.0,huffman
		SAME INPUT t.dat
		COPY OUTPUT t.rz

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

TEST 02 --method + --transform

	EXEC	-m rle.0,huffman -t gzip
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-q
		OUTPUT - '/dev/stdin: <gzip,rle=30,huffman'

	EXEC	-m rle.0,huffman -t gzip
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-q
		OUTPUT - '/dev/stdin: <gzip,rle=30,huffman'

	EXEC	-m rle.0,huffman -t gzip
		SAME INPUT t.dat
		COPY OUTPUT t.rz
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.rz
		SAME OUTPUT t.dat

	EXEC	-q
		OUTPUT - '/dev/stdin: <gzip,rle=30,huffman'
