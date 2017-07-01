# ast nmake interrupt handler tests

INCLUDE test.def

EXIT_USR1=$(( 256 + $(kill -l USR1) ))

TEST 01 'SIGALRM'

	EXEC	--silent
		INPUT Makefile $'all : a
a :
	sleep 11
FREQ = 2s
COUNT = 0
.INIT : .INTERRUPT.INIT
.INTERRUPT.INIT : .MAKE .VIRTUAL .FORCE
	alarm $(FREQ)
.INTERRUPT.ALRM : .MAKE .VIRTUAL .FORCE .REPEAT
	let COUNT = COUNT + 1
	print $(<) $(COUNT)
	alarm $(FREQ)'
		OUTPUT - $'.INTERRUPT.ALRM 1
.INTERRUPT.ALRM 2
.INTERRUPT.ALRM 3
.INTERRUPT.ALRM 4
.INTERRUPT.ALRM 5'

	EXEC	--silent FREQ=3s
		OUTPUT - $'.INTERRUPT.ALRM 1
.INTERRUPT.ALRM 2
.INTERRUPT.ALRM 3'

	EXEC	--silent FREQ=5s
		OUTPUT - $'.INTERRUPT.ALRM 1
.INTERRUPT.ALRM 2'

TEST 02 'obsolete generic interrupt'

	DO	trap - USR1 USR2 # coshell jobs ignore these

	EXEC	--silent --nojobs STATUS=continue
		INPUT Makefile $'.INTERRUPT : .MAKE .VIRTUAL .FORCE .REPEAT
	print $(<:T=M=) : caught signal $(.INTERRUPT)
	.INTERRUPT := $(STATUS)
all : USR1 USR2
USR1 USR2 :
	kill -$(<) $(.SYSCALL getpid)
	sleep 2
.INIT : init
.DONE : done
init done : .MAKE .VIRTUAL .FORCE
	print $(<)'
		OUTPUT - $'init
all : USR1 : .INTERRUPT : caught signal USR1
all : USR2 : .INTERRUPT : caught signal USR2
done'

	EXEC	--silent --nojobs STATUS=exit
		OUTPUT - $'init
all : USR1 : .INTERRUPT : caught signal USR1
done'
		EXIT 3

	EXEC	--silent --nojobs STATUS=signal
		DIAGNOSTICS
		EXIT $EXIT_USR1

TEST 03 'generic interrupt function'

	EXEC	--silent --nojobs STATUS=continue
		INPUT Makefile $'.INTERRUPT : .FUNCTION
	print $(<:T=M=) : caught signal $(%)
	return $(STATUS)
all : USR1 USR2
USR1 USR2 :
	kill -$(<) $(.SYSCALL getpid)
	sleep 1
.INIT : init
.DONE : done
init done : .MAKE .VIRTUAL .FORCE
	print $(<)'
		OUTPUT - $'init
all : USR1 : .INTERRUPT : caught signal USR1
all : USR2 : .INTERRUPT : caught signal USR2
done'

	EXEC	--silent --nojobs STATUS=exit
		OUTPUT - $'init
all : USR1 : .INTERRUPT : caught signal USR1
done'
		EXIT 3

	EXEC	--silent --nojobs STATUS=signal
		DIAGNOSTICS
		EXIT $EXIT_USR1

TEST 04 'obsolete specific interrupt'

	EXEC	--silent --nojobs STATUS=continue
		INPUT Makefile $'.INTERRUPT.USR1 .INTERRUPT.USR2 : .MAKE .VIRTUAL .FORCE .REPEAT
	print $(<:T=M=) : caught signal $(.INTERRUPT)
	.INTERRUPT := $(STATUS)
all : USR1 USR2
USR1 USR2 :
	kill -$(<) $(.SYSCALL getpid)
	sleep 1
.INIT : init
.DONE : done
init done : .MAKE .VIRTUAL .FORCE
	print $(<)'
		OUTPUT - $'init
all : USR1 : .INTERRUPT.USR1 : caught signal USR1
all : USR2 : .INTERRUPT.USR2 : caught signal USR2
done'

	EXEC	--silent --nojobs STATUS=exit
		OUTPUT - $'init
all : USR1 : .INTERRUPT.USR1 : caught signal USR1
done'
		EXIT 3

	EXEC	--silent --nojobs STATUS=signal
		DIAGNOSTICS
		EXIT $EXIT_USR1

TEST 05 'specific interrupt function'

	EXEC	--silent --nojobs STATUS=continue
		INPUT Makefile $'.INTERRUPT.USR1 .INTERRUPT.USR2 : .FUNCTION
	print $(<:T=M=) : caught signal $(%)
	return $(STATUS)
all : USR1 USR2
USR1 USR2 :
	kill -$(<) $(.SYSCALL getpid)
	sleep 1
.INIT : init
.DONE : done
init done : .MAKE .VIRTUAL .FORCE
	print $(<)'
		OUTPUT - $'init
all : USR1 : .INTERRUPT.USR1 : caught signal USR1
all : USR2 : .INTERRUPT.USR2 : caught signal USR2
done'

	EXEC	--silent --nojobs STATUS=exit
		OUTPUT - $'init
all : USR1 : .INTERRUPT.USR1 : caught signal USR1
done'
		EXIT 3

	EXEC	--silent --nojobs STATUS=signal
		DIAGNOSTICS
		EXIT $EXIT_USR1

TEST 06 'multiple interrupts'

	EXEC	-n
		INPUT Makefile $'reap.check = 3s
reset.check = 5m
.GET : .FUNCTION
	local I ID
	ID := $(%:O=1)
	for I $(%:O>1)
		if "$(I:A=.EXISTS|.FAILED)"
			print -u2 $(ID) done $(I)
		else
			eval
			.NOTIFY.$(ID) : .MAKE .VIRTUAL .FORCE .REPEAT .AFTER
				print -u2 $(ID) done $$(<<)
			end
			$(I) : .NOTIFY.$(ID)
			run $(I)
		end
	end
.REAP : .MAKE .VIRTUAL .FORCE .REPEAT
	wait -
	if $(.OUTSTANDING) > 0
		alarm $(reap.check) .REAP
	end
.MAKERUN : .DO.REAP
.DO.REAP : .MAKE .VIRTUAL .FORCE .REPEAT
	alarm $(reap.check) .REAP
.MAKEINIT : .DO.RESET
.DO.RESET : .MAKE .VIRTUAL .FORCE .REPEAT
	reset
	alarm $(reset.check) .DO.RESET
test : test-1 test-2
test-1 : .MAKE
	get 1 x
	get 2 x y
	get 3 x y z
test-2 : .MAKE
	get 4 x
	get 5 y
	get 6 z
x :
	sleep 5
	echo $(<) >&2
y :
	sleep 10
	echo $(<) >&2
z :
	sleep 15
	echo $(<) >&2'
		OUTPUT - $'+ sleep 5
+ echo x >&2
+ sleep 10
+ echo y >&2
+ sleep 15
+ echo z >&2'
		ERROR - $'1 done x
2 done x
2 done y
3 done x
3 done y
3 done z
4 done x
5 done y
6 done z'

	EXEC	--
		OUTPUT -
		ERROR - $'+ sleep 5
+ echo x
+ 1>& 2
x
1 done x
2 done x
3 done x
+ sleep 10
+ echo y
+ 1>& 2
y
2 done y
3 done y
4 done x
5 done y
+ sleep 15
+ echo z
+ 1>& 2
z
3 done z
6 done z'

TEST 07 'continue after caught interrupt'

	EXEC
		INPUT Makefile $'CONTINUE = 1
.INTERRUPT.TERM : .MAKE
	CONTINUE = 0
	print interrupt
.INIT .DONE : .MAKE
	print $(<)
action : .MAKE kill
	print init
	while CONTINUE
	end
	print done
kill :
	set +x; { sleep 1; kill -TERM $(.SYSCALL getpid); } >/dev/null 2>&1 &'
		OUTPUT - $'.INIT
init
interrupt
done
.DONE'
