########################################################################
#                                                                      #
#               This software is part of the ast package               #
#          Copyright (c) 1982-2013 AT&T Intellectual Property          #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#              Information and Software Systems Research               #
#                            AT&T Research                             #
#                           Florham Park NJ                            #
#                                                                      #
#                    David Korn <dgkorn@gmail.com>                     #
#                                                                      #
########################################################################

float DELAY=0.2
integer FOREGROUND=10 BACKGROUND=2

s=$($SHELL -c '
integer i foreground=0 background=0
float delay='$DELAY' d=0 s=0

set --errexit

trap "(( background++ ))" CHLD

(( d = delay ))
for ((i = 0; i < '$BACKGROUND'; i++))
do
    sleep $d &
    (( d *= 4 ))
    (( s += d ))
done

for ((i = 0; i < '$FOREGROUND'; i++))
do
    (( foreground++ ))
    sleep $delay
    (( s -= delay ))
    $SHELL -c : > /dev/null # foreground does not generate SIGCHLD
done

if (( (s += delay) < 1 ))
then
    (( s = 1 ))
fi

sleep $s
wait
print foreground=$foreground background=$background
') || log_error "test loop failed"

eval $s

(( foreground == FOREGROUND )) || log_error "expected '$FOREGROUND foreground' -- got '$foreground' (DELAY=$DELAY)"
(( background == BACKGROUND )) || log_error "expected '$BACKGROUND background' -- got '$background' (DELAY=$DELAY)"

set --noerrexit

if [[ ${.sh.version} == Version?*([[:upper:]])J* ]]
then
    jobmax=4
    got=$($SHELL -c '
        JOBMAX='$jobmax' JOBCOUNT=$(('$jobmax'*2))
        integer running=0 maxrunning=0
        trap "((running--))" CHLD
        for ((i=0; i<JOBCOUNT; i++))
        do
            sleep 1 &
            sleep .1
            if ((++running > maxrunning))
            then
                ((maxrunning=running))
            fi

        done

        wait
        print running=$running maxrunning=$maxrunning
    ')
    exp='running=0 maxrunning='$jobmax
    [[ $got == $exp ]] || log_error "SIGCHLD trap queueing failed -- expected '$exp', got '$got'"

    got=$($SHELL -c '
        typeset -A proc

        trap "
            print \${proc[\$!].name} \${proc[\$!].status} \$?
            unset proc[\$!]
        " CHLD

        { sleep 3; print a; exit 1; } &
        proc[$!]=( name=a status=1 )

        { sleep 2; print b; exit 2; } &
        proc[$!]=( name=b status=2 )

        { sleep 1; print c; exit 3; } &
        proc[$!]=( name=c status=3 )

        while (( ${#proc[@]} ))
        do
            sleep -s
        done
    ')
    exp='c\nc 3 3\nb\nb 2 2\na\na 1 1'
    [[ $got == $exp ]] || log_error "SIGCHLD trap queueing failed -- expected $(printf %q "$exp"), got $(printf %q "$got")"
fi

{
got=$( ( sleep 1;print $'\n') | $SHELL -c 'function handler { : ;}
    trap handler CHLD; sleep .3 & IFS= read; print good')
} 2> /dev/null
[[ $got == good ]] || log_error 'SIGCLD handler effects read behavior'

set -- $(
    (
    $SHELL -xc $'
        trap \'wait $!; print $! $?\' CHLD
        { sleep 0.1; exit 9; } &
        print $!
        sleep 0.5
    '
    ) 2>/dev/null; print $?
)
if (( $# != 4 ))
then
    log_error "CHLD trap failed -- expected 4 args, got $#"
elif (( $4 != 0 ))
then
    log_error "CHLD trap failed -- exit code $4"
elif (( $1 != $2 ))
then
    log_error "child pid mismatch -- got '$1' != '$2'"
elif (( $3 != 9 ))
then
    log_error "child status mismatch -- expected '9', got '$3'"
fi

trap '' CHLD
integer d
for ((d=0; d < 2000; d++))
do
    if print foo | grep bar
    then
        break
    fi
done

(( d==2000 )) ||  log_error "trap '' CHLD  causes side effects d=$d"
trap - CHLD

x=$($SHELL 2> /dev/null -ic '/bin/notfound; sleep .5 & sleep 1;jobs')
[[ $x == *Done* ]] || log_error 'SIGCHLD blocked after notfound'
x=$($SHELL 2> /dev/null  -ic 'kill -0 12345678901234567876; sleep .5 & sleep 1;jobs')
[[ $x == *Done* ]] || log_error 'SIGCHLD blocked after error message'
print 'set -o monitor;sleep .5 & sleep 1;jobs' > $TEST_DIR/foobar
chmod +x $TEST_DIR/foobar
x=$($SHELL  -c "echo | $TEST_DIR/foobar")
[[ $x == *Done* ]] || log_error 'SIGCHLD blocked for script at end of pipeline'

tmpfile=$TEST_DIR/file
$SHELL > $tmpfile <<- \EOF
	trap 'printf "%d %d %s\n" .sh.sig.pid $! "${.sh.sig.code}"' CHLD
	{
		for ((i=0 ; i < 10 ; i++ )) ; do
			sleep .4
		done

	} &
	cpid=$!
	sleep .2 &
	print $cpid $!
	sleep 1
	kill -STOP $cpid
	sleep 1
	kill -CONT $cpid
	sleep 1
	wait
EOF
{
    read xpid pid
    for stat in EXITED STOPPED CONTINUED EXITED
    do
        read pid1 pid2 status  || { log_error "line with stopped continued or exited expected";break;}
        [[ $pid1 == $pid ]] || log_error ".sh.sig.pid=$pid1 should be $pid"
        [[ $pid2 == $pid ]] ||  log_error "\$!=$pid1 should be $pid"
        [[ $status == $stat ]] || log_error "status is $status, should be $stat"
        pid=$xpid
    done

} <  $tmpfile

typeset -A finished
function sighandler_chld
{
    [[ ${finished[${.sh.sig.pid}]} ]] && { log_error "${.sh.sig.pid} already finished and reaped"; trap '' CHLD;}
    finished[${.sh.sig.pid}]=1
}

trap 'sighandler_chld' CHLD
integer i
for (( i=0 ; i < 512 ; i++ ))
do
    sleep 0.1 &
done

wait
