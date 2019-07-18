{ $SHELL -c 'kill %' ;} 2> /dev/null
[[ $? == 1 ]] || log_error "'kill %' has wrong exit status"

wait
unset i
integer i
for (( i=0 ; i < 256 ; i++ ))
do
    sleep 2 &
done

while ! wait
do
    true
done

[[ $(jobs -l) ]] && log_error 'jobs -l should not have any output'

${SHELL} -c 'kill -1 -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -1 -pid not working'
${SHELL} -c 'kill -1 -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -n1 -pid not working'
${SHELL} -c 'kill -s HUP -$$' 2> /dev/null
[[ $(kill -l $?) == HUP ]] || log_error 'kill -HUP -pid not working'

if kill -L > /dev/null 2>&1
then
    [[ $(kill -l HUP) == "$(kill -L HUP)" ]] || log_error 'kill -l and kill -L are not the same when given a signal name'
    [[ $(kill -l 9) == "$(kill -L 9)" ]] || log_error 'kill -l and kill -L are not the same when given a signal number'
    [[ $(kill -L) == *'9) KILL'* ]] || log_error 'kill -L output does not contain 9) KILL'
fi

unset pid1 pid2
false &
pid1=$!
pid2=$(
    wait $pid1
    (( $? == 127 )) || log_error "job known to subshell"
    print $!
)
wait $pid1
(( $? == 1 )) || log_error "wait not saving exit value"
wait $pid2
(( $? == 127 )) || log_error "subshell job known to parent"

# ======
# jobs builtin prints working directory for a job if it does not match current working directory
cat &
# Change working directory
cd
actual=$(jobs)
expect="(pwd: $OLDPWD)"
[[ "$actual" =~ "$expect" ]] || log_error "jobs builtin does not list correct working directory"
kill -15 $!
cd $OLDPWD

# ======
# When interactive mode and job monitoring disabled verify that trying to fg a non-existent job
# produces no output.
actual="${ fg %99 2>&1; }"
expect=""
[[ "$actual" == "$expect" ]] ||
    log_error "fg invalid job with job monitoring disabled" "$expect" "$actual"
