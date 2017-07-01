# : : generated from /home/gsf/src/cmd/builtin/mktemp.rt by mktest : : #

# regression tests for the mktemp command

UNIT mktemp

TEST 01 basics

EXPORT TMPDIR=tmp

	PROG	mkdir tmp
		INPUT -n -
		OUTPUT -
		ERROR -n -

	EXEC	'--regress=12345' --unsafe
		OUTPUT - tmp/tmp5ROunn1.iQo

	EXEC	'--regress=12345' --unsafe tst
		OUTPUT - tst5ROunn1.iQo

	EXEC	'--regress=12345' --unsafe tst.
		OUTPUT - tst.5ROunn1iQo

	EXEC	'--regress=12345' --unsafe tst_
		OUTPUT - tst_5ROunn.iQo

	EXEC	'--regress=12345' --unsafe tstXXXXXXXX
		OUTPUT - tst5ROuiQo8

	EXEC	'--regress=12345' --unsafe tst.XXXXXXXX
		OUTPUT - tst.5ROuiQo8

	EXEC	'--regress=12345' --unsafe tst_XXXXXXXX
		OUTPUT - tst_5ROuiQo8

	EXEC	'--regress=12345' --unsafe --tmp
		OUTPUT - tmp/tmp5ROunn1.iQo

	EXEC	'--regress=12345' --unsafe --tmp tst
		OUTPUT - tmp/tst5ROunn1.iQo

	EXEC	'--regress=12345' --unsafe --tmp tmp/tst

	EXEC	'--regress=12345' --unsafe --tmp tst.
		OUTPUT - tmp/tst.5ROunn1iQo

	EXEC	'--regress=12345' --unsafe --tmp tmp/tst.

	EXEC	'--regress=12345' --unsafe --tmp tst_
		OUTPUT - tmp/tst_5ROunn.iQo

	EXEC	'--regress=12345' --unsafe --tmp tmp/tst_

	EXEC	'--regress=12345' --unsafe --tmp tstXXXXXXXX
		OUTPUT - tmp/tst5ROuiQo8

	EXEC	'--regress=12345' --unsafe --tmp tmp/tstXXXXXXXX

	EXEC	'--regress=12345' --unsafe --tmp tst.XXXXXXXX
		OUTPUT - tmp/tst.5ROuiQo8

	EXEC	'--regress=12345' --unsafe --tmp tmp/tst.XXXXXXXX

	EXEC	'--regress=12345' --unsafe --tmp tst_XXXXXXXX
		OUTPUT - tmp/tst_5ROuiQo8

	EXEC	'--regress=12345' --unsafe --tmp tmp/tst_XXXXXXXX
