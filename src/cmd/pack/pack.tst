# regression tests for the pack utilitiy

VIEW data pack.dat

TEST 01 'basics'
	DO cp $data .
	EXEC	pack.dat
		OUTPUT - $'pack: pack.dat : 31.8% Compression'
	PROG	unpack pack.dat
		OUTPUT - $'unpack: pack.dat.z: unpacked'
	PROG	cmp $data pack.dat
		OUTPUT -
