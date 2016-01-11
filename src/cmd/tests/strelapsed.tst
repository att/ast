# regression tests for the ast strelapsed() routine

TEST 01 'basics'
	EXEC	1 '1h5s' 1 '1hour5sec' 1 '1 hour 5 sec'
		OUTPUT - $'strelapsed   "1h5s" "" 3605 1
strelapsed   "1hour5sec" "" 3605 1
strelapsed   "1 hour 5 sec" "" 3605 1'
	EXEC	10 '1h5s' 10 '1hour5sec' 10 '1 hour 5 sec'
		OUTPUT - $'strelapsed   "1h5s" "" 36050 10
strelapsed   "1hour5sec" "" 36050 10
strelapsed   "1 hour 5 sec" "" 36050 10'
	EXEC	1 '3m5d' 1 '3mi5da' 1 '3M5da' 1 '3mo5da' 1 '3MI5da' 1 '3Mi5da' 1 '3Mo5da' 1 '3MO5da'
		OUTPUT - $'strelapsed   "3m5d" "" 432180 1
strelapsed   "3mi5da" "" 432180 1
strelapsed   "3M5da" "" 7689600 1
strelapsed   "3mo5da" "" 7689600 1
strelapsed   "3MI5da" "" 432180 1
strelapsed   "3Mi5da" "" 432180 1
strelapsed   "3Mo5da" "" 7689600 1
strelapsed   "3MO5da" "" 7689600 1'
