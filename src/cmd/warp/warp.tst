# tests for the warp command

KEEP "*.dat"

format=%Y-%m-%d+%H:%M

huh0=1999-09-01+12:34
huh1=2000-01-28+00:00
huh2=2000-02-01+00:00

y2k0=1999-12-31+23:59
y2k1=2000-01-01+00:00
y2k2=2000-01-01+00:01

leap0=2000-02-28+00:00
leap1=2000-02-29+00:00
leap2=2000-03-01+00:00

TEST 01 'date'
	EXEC	$huh0 date -f $format
		OUTPUT - $huh0
	EXEC	$huh0 $SHELL -c "date -f $format"
	EXEC	$huh1 date -f $format
		OUTPUT - $huh1
	EXEC	$huh2 date -f $format
		OUTPUT - $huh2
	EXEC	$huh2 $SHELL -c "date -f $format"
	EXEC	$y2k0 date -f $format
		OUTPUT - $y2k0
	EXEC	$y2k0 $SHELL -c "date -f $format"
	EXEC	$y2k1 date -f $format
		OUTPUT - $y2k1
	EXEC	$y2k2 date -f $format
		OUTPUT - $y2k2
	EXEC	$y2k2 $SHELL -c "date -f $format"
	EXEC	$leap0 date -f $format
		OUTPUT - $leap0
	EXEC	$leap0 $SHELL -c "date -f $format"
	EXEC	$leap1 date -f $format
		OUTPUT - $leap1
	EXEC	$leap1 $SHELL -c "date -f $format"
	EXEC	$leap2 date -f $format
		OUTPUT - $leap2
	EXEC	$leap2 $SHELL -c "date -f $format"

TEST 02 'touch'
	EXEC	$leap0 touch -t ${leap0#*+}:30 t
		OUTPUT -
	EXEC	$leap0 date -f $format -m t
		OUTPUT - $leap0
	EXEC	$leap0 $SHELL -c "touch -t ${leap0#*+}:30 t"
		OUTPUT -
	EXEC	$leap0 date -f $format -m t
		OUTPUT - $leap0
	EXEC	$leap0 $SHELL -c "date -f $format -m t"
	EXEC	$leap1 touch -t ${leap1#*+}:30 t
		OUTPUT -
	EXEC	$leap1 $SHELL -c "touch -t ${leap1#*+}:30 t"
	EXEC	$leap1 date -f $format -m t
		OUTPUT - $leap1
	EXEC	$leap1 $SHELL -c "date -f $format -m t"
	EXEC	$leap2 touch -t ${leap2#*+}:30 t
		OUTPUT -
	EXEC	$leap2 $SHELL -c "touch -t ${leap2#*+}:30 t"
	EXEC	$leap2 date -f $format -m t
		OUTPUT - $leap2
	EXEC	$leap2 $SHELL -c "date -f $format -m t"
	EXEC	$leap0 touch -t ${leap0#*+}:30 t
		OUTPUT -
	EXEC	$leap0 $SHELL -c "touch -t ${leap0#*+}:30 t"
	EXEC	$leap0 date -f $format -m t
		OUTPUT - $leap0
	EXEC	$leap0 $SHELL -c "date -f $format -m t"
	EXEC	$leap1 touch -t ${leap1#*+}:30 t
		OUTPUT -
	EXEC	$leap1 $SHELL -c "touch -t ${leap1#*+}:30 t"
	EXEC	$leap1 date -f $format -m t
		OUTPUT - $leap1
	EXEC	$leap1 $SHELL -c "date -f $format -m t"
	EXEC	$leap2 touch -t ${leap2#*+}:30 t
		OUTPUT -
	EXEC	$leap2 $SHELL -c "touch -t ${leap2#*+}:30 t"
	EXEC	$leap2 date -f $format -m t
		OUTPUT - $leap2
	EXEC	$leap2 $SHELL -c "date -f $format -m t"
