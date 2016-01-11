# regression tests for the rtable vcodex plugin

UNIT vczip

TEST 01 basics

	EXEC	-m rtable^mtf^rle.0^huffpart
		INPUT t.dat 'hello world'
		SAME INPUT t.dat
		COPY OUTPUT t.tz
		IGNORE OUTPUT

	EXEC	 -u
		SAME INPUT t.tz
		SAME OUTPUT t.dat
