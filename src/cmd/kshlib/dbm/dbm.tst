# : : generated from dbm.rt by mktest : : #

# regression tests for the ksh dbm plugin

UNIT ksh

TEST 01 '--read, --write'

	EXEC	-c $'builtin -f dbm dbm_open dbm_get dbm_set dbm_close
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
\t\tdbm_open --write tst.dbm
\t\twhile\t(( $# >= 2 ))
\t\tdo\tdbm_set "$1" "$2"
\t\t\tshift 2
\t\tdone
\t\tdbm_close
\t'

	EXEC	-c $'builtin -f dbm dbm_open dbm_get dbm_set dbm_close
\t\tdbm_open --read tst.dbm
\t\tset -s -- $(
\t\t\twhile\t:
\t\t\tdo\tkey=$(dbm_get)
\t\t\t\t[[ $key ]] || break
\t\t\t\tprint -r -- "$key"
\t\t\tdone
\t\t)
\t\tprint -r "$@"
\t\tdbm_close
\t'
		OUTPUT - 'aaa bbb ccc ppp qqq rrr xxx yyy zzz'

	EXEC	-c $'builtin -f dbm dbm_open dbm_get dbm_set dbm_close
\t\tdbm_open --read tst.dbm
\t\tset -s -- $(
\t\t\twhile\t:
\t\t\tdo\tkey=$(dbm_get)
\t\t\t\t[[ $key ]] || break
\t\t\t\tprint -r -- "$key"
\t\t\tdone
\t\t)
\t\tfor key
\t\tdo\tprint -r -- "$key=$(dbm_get "$key")"
\t\tdone
\t\tdbm_close
\t'
		OUTPUT - aaa=$'111\nbbb=222\nccc=333\nppp=444\nqqq=555\nrrr=666\nxxx=777\nyyy=888\nzzz=999'

	EXEC	-c $'builtin -f dbm dbm_open dbm_get dbm_set dbm_close
\t\tdbm_open --read --write tst.dbm
\t\tset -s -- $(
\t\t\twhile\t:
\t\t\tdo\tkey=$(dbm_get)
\t\t\t\t[[ $key ]] || break
\t\t\t\tprint -r -- "$key"
\t\t\tdone
\t\t)
\t\tfor key
\t\tdo\tdbm_set "$key" "$(( $(dbm_get "$key") + 1000 ))"
\t\tdone
\t\tfor key
\t\tdo\tprint -r -- "$key=$(dbm_get "$key")"
\t\tdone
\t\tdbm_close
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
