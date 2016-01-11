# : : generated from dbm_t.rt by mktest : : #

# regression tests for the ksh dbm_t plugin

UNIT ksh

TEST 01 '--create, --read, --write'

	EXEC	-c $'builtin -f dbm_t
\t\tset \\
\t\t\taaa\t111 \\
\t\t\tbbb\t222 \\
\t\t\tccc\t333 \\
\t\t\tppp\t444 \\
\t\t\tqqq\t555 \\
\t\t\trrr\t666 \\
\t\t\txxx\t777 \\
\t\t\tyyy\t888 \\
\t\t\tzzz\t999
\t\tDbm_t --create tst=tst.dbm
\t\twhile\t(( $# >= 2 ))
\t\tdo\ttst[$1]=$2
\t\t\tshift 2
\t\tdone
\t'

	EXEC	-c $'builtin -f dbm_t
\t\tDbm_t tst=tst.dbm
\t\tset -s -- ${!tst[@]}
\t\tfor key
\t\tdo\tprint -r -- "$key=${tst[$key]}"
\t\tdone
\t'
		OUTPUT - aaa=$'111\nbbb=222\nccc=333\nppp=444\nqqq=555\nrrr=666\nxxx=777\nyyy=888\nzzz=999'

	EXEC	-c $'builtin -f dbm_t
\t\tDbm_t --write tst=tst.dbm
\t\tset -s -- ${!tst[@]}
\t\tfor key
\t\tdo\ttst[$key]=$(( ${tst[$key]} + 1000 ))
\t\tdone
\t\tfor key
\t\tdo\tprint -r -- "$key=${tst[$key]}"
\t\tdone
\t'
		OUTPUT - $'aaa=1111
bbb=1222
ccc=1333
ppp=1444
qqq=1555
rrr=1666
xxx=1777
yyy=1888
zzz=1999'

TEST 02 '--zero + --create, --read, --write'

	EXEC	-c $'builtin -f dbm_t
\t\tset \\
\t\t\taaa\t111 \\
\t\t\tbbb\t222 \\
\t\t\tccc\t333 \\
\t\t\tppp\t444 \\
\t\t\tqqq\t555 \\
\t\t\trrr\t666 \\
\t\t\txxx\t777 \\
\t\t\tyyy\t888 \\
\t\t\tzzz\t999
\t\tDbm_t --create --zero tst=tst.dbm
\t\twhile\t(( $# >= 2 ))
\t\tdo\ttst[$1]=$2
\t\t\tshift 2
\t\tdone
\t'

	EXEC	-c $'builtin -f dbm_t
\t\tDbm_t tst=tst.dbm
\t\tset -s -- ${!tst[@]}
\t\tfor key
\t\tdo\tprint -r -- "$key=${tst[$key]}"
\t\tdone
\t'
		OUTPUT - aaa=$'111\nbbb=222\nccc=333\nppp=444\nqqq=555\nrrr=666\nxxx=777\nyyy=888\nzzz=999'

	EXEC	-c $'builtin -f dbm_t
\t\tDbm_t --write tst=tst.dbm
\t\tset -s -- ${!tst[@]}
\t\tfor key
\t\tdo\ttst[$key]=$(( ${tst[$key]} + 1000 ))
\t\tdone
\t\tfor key
\t\tdo\tprint -r -- "$key=${tst[$key]}"
\t\tdone
\t'
		OUTPUT - $'aaa=1111
bbb=1222
ccc=1333
ppp=1444
qqq=1555
rrr=1666
xxx=1777
yyy=1888
zzz=1999'
