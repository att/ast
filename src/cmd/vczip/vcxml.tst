# regression tests for the xml vcodex plugin

UNIT vczip

VIEW data

TEST 01 basics

	EXEC	-m xml
		SAME INPUT $data/t.xml
		COPY OUTPUT t.x
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.x
		SAME OUTPUT $data/t.xml

TEST 02 basics

	EXEC	-m xml^gzip
		SAME INPUT $data/t.xml
		COPY OUTPUT t.x.g
		IGNORE OUTPUT

	EXEC	-u
		SAME INPUT t.x.g
		SAME OUTPUT $data/t.xml
