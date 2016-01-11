# : : generated from mktemp.rt by mktest : : #

# regression tests for the mktemp command

UNIT mktemp

TEST 01 basics

EXPORT TMPDIR=tmp

	PROG	mkdir tmp
		INPUT -n -
		OUTPUT -
		ERROR -n -

	EXEC	--regress=12345 --unsafe
		OUTPUT - tmp/tmp_1h.u8h

	EXEC	--regress=12345 --unsafe tst
		OUTPUT - tst1h.u8h

	EXEC	--regress=12345 --unsafe tst.
		OUTPUT - tst.1h8u8h

	EXEC	--regress=12345 --unsafe tst_
		OUTPUT - tst_1h.u8h

	EXEC	--regress=12345 --unsafe tstXXXXXXXX
		OUTPUT - tst1h8ou8h7

	EXEC	--regress=12345 --unsafe tst.XXXXXXXX
		OUTPUT - tst.1h8ou8h7

	EXEC	--regress=12345 --unsafe tst_XXXXXXXX
		OUTPUT - tst_1h8ou8h7

	EXEC	--regress=12345 --unsafe --tmp
		OUTPUT - tmp/tmp_1h.u8h

	EXEC	--regress=12345 --unsafe --tmp tst
		OUTPUT - tmp/tst1h.u8h

	EXEC	--regress=12345 --unsafe --tmp tmp/tst

	EXEC	--regress=12345 --unsafe --tmp tst.
		OUTPUT - tmp/tst.1h8u8h

	EXEC	--regress=12345 --unsafe --tmp tmp/tst.

	EXEC	--regress=12345 --unsafe --tmp tst_
		OUTPUT - tmp/tst_1h.u8h

	EXEC	--regress=12345 --unsafe --tmp tmp/tst_

	EXEC	--regress=12345 --unsafe --tmp tstXXXXXXXX
		OUTPUT - tmp/tst1h8ou8h7

	EXEC	--regress=12345 --unsafe --tmp tmp/tstXXXXXXXX

	EXEC	--regress=12345 --unsafe --tmp tst.XXXXXXXX
		OUTPUT - tmp/tst.1h8ou8h7

	EXEC	--regress=12345 --unsafe --tmp tmp/tst.XXXXXXXX

	EXEC	--regress=12345 --unsafe --tmp tst_XXXXXXXX
		OUTPUT - tmp/tst_1h8ou8h7

	EXEC	--regress=12345 --unsafe --tmp tmp/tst_XXXXXXXX
