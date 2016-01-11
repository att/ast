# tests for the dss shell type library

TITLE + type

export LC_ALL=C TZ=EST5EDT

builtin -f dss
dss.load time_t
typeset -T dss.time_t

function part
{
	time_t t="$1"
	while	(( $# > 1 ))
	do	shift
		eval "$1"
	done
	print -- $t ${t.year}-${t.mon}-${t.mday}+${t.hour}:${t.min}:${t.sec} ${t.wday}
}

TEST 01 'basics'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'2003-01-02+03:04:05 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 t.year=2004
		OUTPUT - $'2004-01-02+03:04:05 2004-January-2+3:4:5 Friday'
	PROG part 2003-01-02+03:04:05 t.mon=2
		OUTPUT - $'2003-02-02+03:04:05 2003-February-2+3:4:5 Sunday'
	PROG part 2003-01-02+03:04:05 t.mday=3
		OUTPUT - $'2003-01-03+03:04:05 2003-January-3+3:4:5 Friday'
	PROG part 2003-01-02+03:04:05 t.hour=4
		OUTPUT - $'2003-01-02+04:04:05 2003-January-2+4:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 t.min=5
		OUTPUT - $'2003-01-02+03:05:05 2003-January-2+3:5:5 Thursday'
	PROG part 2003-01-02+03:04:05 t.sec=6
		OUTPUT - $'2003-01-02+03:04:06 2003-January-2+3:4:6 Thursday'
	PROG part 2003-01-02+03:04:05 t.wday=6
		OUTPUT - $'2003-01-03+03:04:05 2003-January-3+3:4:5 Friday'
	PROG part 2003-01-02+03:04:05 t.mon=March
		OUTPUT - $'2003-03-02+03:04:05 2003-March-2+3:4:5 Sunday'
	PROG part 2003-01-02+03:04:05 t.wday=Tuesday
		OUTPUT - $'2003-01-07+03:04:05 2003-January-7+3:4:5 Tuesday'

TEST 02 'local variable details override'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'2003-01-02+03:04:05 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 "t._=%c"
		OUTPUT - $'Thu Jan 2 03:04:05 2003 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 "t.mon._=%02d" t.mon=2
		OUTPUT - $'2003-02-02+03:04:05 2003-02-2+3:4:5 Sunday'
	PROG part 2003-01-02+03:04:05 "t.hour._=%s" t.hour=4
		OUTPUT - $'2003-01-02+04:04:05 2003-January-2+4AM:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 "t.hour._=%s" t.hour=20
		OUTPUT - $'2003-01-02+20:04:05 2003-January-2+8PM:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 "t.wday._=%d" t.wday=6
		OUTPUT - $'2003-01-03+03:04:05 2003-January-3+3:4:5 6'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'2003-01-02+03:04:05 2003-January-2+3:4:5 Thursday'

TEST 03 'global type details override'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'2003-01-02+03:04:05 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 ".sh.type.dss.time_t._=%c"
		OUTPUT - $'Thu Jan 2 03:04:05 2003 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'Thu Jan 2 03:04:05 2003 2003-January-2+3:4:5 Thursday'
	PROG part 2003-01-02+03:04:05 ".sh.type.dss.tm_mon_t._=%d" ".sh.type.dss.tm_wday_t._=%02d" ".sh.type.dss.tm_hour_t._=%02d"
		OUTPUT - $'Thu Jan 2 03:04:05 2003 2003-1-2+03:4:5 05'
	PROG part 2003-01-02+03:04:05 t.mon=2
		OUTPUT - $'Sun Feb 2 03:04:05 2003 2003-2-2+03:4:5 01'
	PROG part 2003-01-02+03:04:05 t.hour=4
		OUTPUT - $'Thu Jan 2 04:04:05 2003 2003-1-2+04:4:5 05'
	PROG part 2003-01-02+03:04:05 t.hour=20
		OUTPUT - $'Thu Jan 2 20:04:05 2003 2003-1-2+20:4:5 05'
	PROG part 2003-01-02+03:04:05 t.wday=6
		OUTPUT - $'Fri Jan 3 03:04:05 2003 2003-1-3+03:4:5 06'
	PROG part 2003-01-02+03:04:05
		OUTPUT - $'Thu Jan 2 03:04:05 2003 2003-1-2+03:4:5 05'
