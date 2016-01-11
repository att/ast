# regression tests for the ama vcodex plugin

UNIT vczip

TEST 01 basics

	EXEC	 -mqv
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.qz
		IGNORE OUTPUT

	EXEC	 -u
		SAME INPUT t.qz
		SAME OUTPUT t.dat

	EXEC	 -mqv
		SAME INPUT t.dat
		COPY OUTPUT t.qz

	EXEC	 -u
		SAME INPUT t.qz
		SAME OUTPUT t.dat

	EXEC	 -m qv
		SAME INPUT t.dat
		COPY OUTPUT t.qz

	EXEC	 -u
		SAME INPUT t.qz
		SAME OUTPUT t.dat

	EXEC	 -m ama,transpose,rle,huffman
		SAME INPUT t.dat
		COPY OUTPUT t.qz

	EXEC	 -u
		SAME INPUT t.qz
		SAME OUTPUT t.dat

	EXEC	 -m ama^transpose^rle^huffman
		SAME INPUT t.dat
		COPY OUTPUT t.qz

	EXEC	 -u
		SAME INPUT t.qz
		SAME OUTPUT t.dat
