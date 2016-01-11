# regression tests for the touch utility

export TZ=EST5EDT

TEST 01 'basics'
	EXEC -a -m -t 2000-01-02+03:04:05 a
	PROG ls --format="%(atime:time=%K)s %(mtime:time=%K)s %(path)s" a
		OUTPUT - $'2000-01-02+03:04:05 2000-01-02+03:04:05 a'
	EXEC -a -t 2000-05-04+03:02:01 a
		OUTPUT -
	PROG ls --format="%(atime:time=%K)s %(mtime:time=%K)s %(path)s" a
		OUTPUT - $'2000-05-04+03:02:01 2000-01-02+03:04:05 a'
	EXEC -r a b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-01-02+03:04:05 b'
	EXEC -a -r a b
		OUTPUT -
	PROG ls --format="%(atime:time=%K)s %(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-05-04+03:02:01 2000-01-02+03:04:05 b'
	EXEC -n b
		OUTPUT -
	PROG ls --format="%(atime:time=%K)s %(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-05-04+03:02:01 2000-01-02+03:04:05 b'

TEST 02 'times'
	EXEC -a -t 2000-05-04+03:02:01 a
	EXEC -m -t 2000-01-02+03:04:05 a
	EXEC -a -t mtime a
	PROG ls --format="%(atime:time=%K)s %(mtime:time=%K)s %(path)s" a
		OUTPUT - $'2000-01-02+03:04:05 2000-01-02+03:04:05 a'
	EXEC 0102030405 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2001-02-03+04:05:00 b'
	EXEC -t 0102030405 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2001-02-03+04:05:00 b'
	EXEC 0203040501 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2002-03-04+05:01:00 b'
	EXEC -t 0203040501 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2002-03-04+05:01:00 b'
	EXEC 9901020304 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'1999-01-02+03:04:00 b'
	EXEC -t 9901020304 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'1999-01-02+03:04:00 b'
	EXEC 0102030499 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'1999-01-02+03:04:00 b'
	EXEC -t 0102030499 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'1999-01-02+03:04:00 b'
	EXEC 200001020304 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-01-02+03:04:00 b'
	EXEC -t 200001020304 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-01-02+03:04:00 b'
	EXEC 010203042000 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-01-02+03:04:00 b'
	EXEC -t 010203042000 b
		OUTPUT -
	PROG ls --format="%(mtime:time=%K)s %(path)s" b
		OUTPUT - $'2000-01-02+03:04:00 b'

TEST 03 'subsecond times'

	IF 'touch -a -t 2000-05-04+03:02:01.098 a; [[ $(ls --format="%(atime:time=%N)s" a) == 098000000 ]]'

	EXEC -a -t 2000-05-04+03:02:01.098 a
	EXEC -m -t 2000-01-02+03:04:05.678 a
	PROG ls --format="%(atime:time=%K.%N)s %(mtime:time=%K.%N)s %(path)s" a
		OUTPUT - $'2000-05-04+03:02:01.098000000 2000-01-02+03:04:05.678000000 a'
	EXEC -a -t mtime a
		OUTPUT -
	PROG ls --format="%(atime:time=%K.%N)s %(mtime:time=%K.%N)s %(path)s" a
		OUTPUT - $'2000-01-02+03:04:05.678000000 2000-01-02+03:04:05.678000000 a'

	ELSE 'filesystem disables subsecond time stamps'

	FI
