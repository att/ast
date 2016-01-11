# regression tests for the bzip2 utility

VIEW data

TEST 01 'samples'
	EXEC	-zc -1 $data/sample1.ref
		SAME OUTPUT $data/sample1.bz2
	EXEC	-zc -2 $data/sample2.ref
		SAME OUTPUT $data/sample2.bz2
	EXEC	-dc $data/sample1.bz2
		SAME OUTPUT $data/sample1.ref
	EXEC	-dc $data/sample2.bz2
		SAME OUTPUT $data/sample2.ref
