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
########################################################################
#                                                                      #
#               This software is part of the ast package               #
#                 Copyright (c) 2009-2012 Roland Mainz                 #
#                      and is licensed under the                       #
#                 Eclipse Public License, Version 1.0                  #
#                    by AT&T Intellectual Property                     #
#                                                                      #
#                A copy of the License is available at                 #
#          http://www.eclipse.org/org/documents/epl-v10.html           #
#         (with md5 checksum b35adb5213ca9657e911e9befb180842)         #
#                                                                      #
#                                                                      #
#                 Roland Mainz <roland.mainz@nrubsig.org>              #
#                                                                      #
########################################################################

#
# Copyright (c) 2009, 2012, Roland Mainz. All rights reserved.
#

#
# Test module to check the ksh93 "poll" builtin
#

# test setup
function err_exit
{
	print -u2 -n '\t'
	print -u2 -r ${Command}[$1]: "${@:2}"
	(( Errors++ ))
}
alias err_exit='err_exit $LINENO'

set -o nounset
Command=${0##*/}
integer Errors=0

typeset ocwd
typeset tmpdir

# create temporary test directory
ocwd="${PWD}"
tmpdir="${ mktemp -t -d 'test_builtin_poll.XXXXXXXX' ; }" || err_exit 'Cannot create temporary directory.'

cd "${tmpdir}" || { err_exit "cd ${tmpdir} failed." ; exit $((Errors<125?Errors:125)) ; }


# basic tests
function test1
{
	compound d1=(
		compound -A u=(
			[y]=( integer fd=5 ; compound events=( pollin='true' ) revents=() )
			[x]=( integer fd=5 ; compound events=( pollin='true' ) revents=() )
		)
	)

	# test 1:
	print -C d1 >'testdata.cpv'
	cat '/dev/zero' | $SHELL -o errexit -c 'builtin poll ; read -C <"testdata.cpv" d1 ; redirect 5<&0 ; poll -e d1.res -t 5. d1.u ; print -C d1 >"testdata.cpv"' >'log.txt' 2>&1 || err_exit "poll returned non-zero exit code $?"
	unset d1 ; read -C d1 <'testdata.cpv' || err_exit 'Cannot read test data.'
	[[ "$(< 'log.txt')" == '' ]] || err_exit "Excepted empty stdout/stderr, got $(printf '%q\n' "$(< 'log.txt')")"
	[[ "${d1.u[x].revents.pollin-}" == 'true' ]] || err_exit "d1.u[x].revents contains '${d1.u[x].revents-}', not 'POLLIN'"
	[[ "${d1.u[y].revents.pollin-}" == 'true' ]] || err_exit "d1.u[y].revents contains '${d1.u[y].revents-}', not 'POLLIN'"
	[[ "${d1.res[*]-}" == 'x y' ]] || err_exit "d1.res contains '${d1.res[*]-}', not 'x y'"

	rm 'testdata.cpv' 'log.txt'

	# test 2:
	unset d1.res
	d1.u[z]=( integer fd=5 ; compound events=( pollout='true' ) revents=() )

	print -C d1 >'testdata.cpv'
	$SHELL -o errexit -c 'builtin poll ; read -C <"testdata.cpv" d1 ; { poll -e d1.res -t 5. d1.u ; } 5<"/dev/null" 5>"/dev/null" ; print -C d1 >"testdata.cpv"' >'log.txt' 2>&1 || err_exit "poll returned non-zero exit code $?"
	unset d1 ; read -C d1 <'testdata.cpv' || err_exit 'Cannot read test data.'

	[[ "$(< 'log.txt')" == '' ]] || err_exit "Excepted empty stdout/stderr, got $(printf '%q\n' "$(< 'log.txt')")"
	[[ "${d1.u[x].revents.pollin-}"  == 'true' ]] || err_exit "d1.u[x].revents contains '${d1.u[x].revents-}', not 'POLLIN'"
	[[ "${d1.u[y].revents.pollin-}"  == 'true' ]] || err_exit "d1.u[y].revents contains '${d1.u[y].revents-}', not 'POLLIN'"
	[[ "${d1.u[z].revents.pollout-}" == 'true' ]] || err_exit "d1.u[z].revents contains '${d1.u[z].revents-}', not 'POLLOUT,'"
	[[ "${d1.res[*]-}" == 'x y z' ]] || err_exit "d1.res contains '${d1.res[*]-}', not 'x y z'"

	rm 'testdata.cpv' 'log.txt'

	return 0
}

function test_sparse_array1
{
	compound out=( typeset stdout stderr ; integer res )
	integer i
	typeset expected_output_pattern
	typeset testname
	compound -r -a tests=(
		(
			name='sparse_compound_array1'
			typeset -r -a script=(
				$'builtin poll\n'
				$'compound -a pl\n'
				$'integer i maxfd=$(getconf "OPEN_MAX")\n'
				$'(( maxfd-=10 )) # space for stdin/stdout/stderr/dirfd\n'
				$'for (( i=0 ; i < maxfd ; i++ )) ; do\n'
				$'	pl[$((i*17))]=( fd=0 events=( pollin=true ) )\n'
				$'done\n'
				$'poll -R -t2 pl\n'
				$'print -v pl\n'
				$'print "OK"\n'
			)
			expected_output_pattern='~(E)([[:space:]]*?\([[:space:]]*?\[.+\]=\([[:space:]]*?events=\([[:space:]]*?pollin=true[[:space:]]*?\)[[:space:]]*?fd=0[[:space:]]*?revents=\([[:space:]]*?pollhup=true[[:space:]]*?pollin=true[[:space:]]*?\)[[:space:]]*?\)[[:space:]]*?\)[[:space:]]*?)+OK'
		)
		(
			name='sparse_type_array1'
			typeset -r -a script=(
				$'builtin poll\n'
				$'typeset -T p_t=( typeset -l -i fd ; \n'
				# fixme: In theory we only have to list the events we want
				# in the compound variable "events" ... and the same events
				# plus pollerr, pollnval and pollup in "revents" (since
				# they may appear at any time).
				# Unfortunately ksh93v- has a bug in the type system
				# which triggers fatal errors in case we try to read a
				# non-existing type variable member - see
				# http://marc.info/?l=ast-developers&m=134526131905059&w=2
				$'	compound events=(  typeset pollin=false pollpri=false pollout=false pollrdnorm=false pollwrnorm=false pollrdband=false pollwrband=false pollmsg=false pollremove=false pollrdhup=false pollerr=false pollhup=false pollnval=false ; )\n'
				$'	compound revents=( typeset pollin=false pollpri=false pollout=false pollrdnorm=false pollwrnorm=false pollrdband=false pollwrband=false pollmsg=false pollremove=false pollrdhup=false pollerr=false pollhup=false pollnval=false ; )\n'
				$'	function pinit { _.fd=$1 ; _.events.pollin=true ; } ;\n'
				$')\n'
				$'compound c ; p_t -a c.pl\n'
				$'integer i maxfd=$(getconf "OPEN_MAX")\n'
				# fixme: fmin(maxfd, 16) is used to prevent ~(Xlr) below from
				# consuming too much stack. Future versions of "shtests" will
				# enfore ulimit -s 65536 and therefore avoid this mess
				$'(( maxfd=fmin(maxfd, 16) , maxfd-=10 )) # space for stdin/stdout/stderr/dirfd\n'
				$'for (( i=0 ; i < maxfd ; i++ )) ; do\n'
				$'	c.pl[$((i*17))].pinit 0\n'
				$'done\n'
				$'poll -R -t2 c.pl\n'
				$'print -v c\n'
				$'print "OK"\n'
			)
			expected_output_pattern='~(Xlr)\([[:space:]]+?p_t[[:space:]]+?-a[[:space:]]+?pl=\([[:space:]]+?([[:space:]]+?\[[[:alnum:]]+\]=\([[:space:]]+?typeset[[:space:]]+?-l[[:space:]]+?-i[[:space:]]+?fd=0[[:space:]]+?events=\((([^\x28\x29]+?pollin=true[^\x28\x29]+?)&([^\x28\x29]+?pollhup=false[^\x28\x29]+?))[[:space:]]+?\)[[:space:]]+?revents=\((([^\x28\x29]+?pollin=true[^\x28\x29]+?)&([^\x28\x29]+?pollhup=true[^\x28\x29]+?))\)[[:space:]]+?\)[[:space:]]+?)+?[[:space:]]+?\)[[:space:]]+?\)[[:space:]]+?OK'
		)
		(
			name='associative_type_array1'
			typeset -r -a script=(
				$'builtin poll\n'
				$'typeset -T p_t=( typeset -l -i fd ; \n'
				# fixme: In theory we only have to list the events we want
				# in the compound variable "events" ... and the same events
				# plus pollerr, pollnval and pollup in "revents" (since
				# they may appear at any time).
				# Unfortunately ksh93v- has a bug in the type system
				# which triggers fatal errors in case we try to read a
				# non-existing type variable member - see
				# http://marc.info/?l=ast-developers&m=134526131905059&w=2
				$'	compound events=(  typeset pollin=false pollpri=false pollout=false pollrdnorm=false pollwrnorm=false pollrdband=false pollwrband=false pollmsg=false pollremove=false pollrdhup=false pollerr=false pollhup=false pollnval=false ; )\n'
				$'	compound revents=( typeset pollin=false pollpri=false pollout=false pollrdnorm=false pollwrnorm=false pollrdband=false pollwrband=false pollmsg=false pollremove=false pollrdhup=false pollerr=false pollhup=false pollnval=false ; )\n'
				$'	function pinit { _.fd=$1 ; _.events.pollin=true ; } ;\n'
				$')\n'
				$'compound c ; p_t -A c.pl\n'
				$'integer i maxfd=$(getconf "OPEN_MAX")\n'
				# fixme: fmin(maxfd, 16) is used to prevent ~(Xlr) below from
				# consuming too much stack. Future versions of "shtests" will
				# enfore ulimit -s 65536 and therefore avoid this mess
				$'(( maxfd=fmin(maxfd, 16) , maxfd-=10 )) # space for stdin/stdout/stderr/dirfd\n'
				$'for (( i=0 ; i < maxfd ; i++ )) ; do\n'
				$'	c.pl[$((i*17))].pinit 0\n'
				$'done\n'
				$'poll -R -t2 c.pl\n'
				$'print -v c\n'
				$'print "OK"\n'
			)
			expected_output_pattern='~(Xlr)\([[:space:]]+?p_t[[:space:]]+?-A[[:space:]]+?pl=\([[:space:]]+?([[:space:]]+?\[[[:alnum:]]+\]=\([[:space:]]+?typeset[[:space:]]+?-l[[:space:]]+?-i[[:space:]]+?fd=0[[:space:]]+?events=\((([^\x28\x29]+?pollin=true[^\x28\x29]+?)&([^\x28\x29]+?pollhup=false[^\x28\x29]+?))[[:space:]]+?\)[[:space:]]+?revents=\((([^\x28\x29]+?pollin=true[^\x28\x29]+?)&([^\x28\x29]+?pollhup=true[^\x28\x29]+?))\)[[:space:]]+?\)[[:space:]]+?)+?[[:space:]]+?\)[[:space:]]+?\)[[:space:]]+?OK'
		)
	)

	for (( i=0 ; i < ${#tests[@]} ; i++ )) ; do
		nameref tst=tests[i]
		testname="${0}/${i}/${tst.name}"
		expected_output_pattern="${tst.expected_output_pattern}"

		out.stderr="${ { out.stdout="${ print 'IN' | ${SHELL} -o nounset -c "${tst.script[*]}" ; (( out.res=$? )) ; }" ; } 2>&1 ; }"

		[[ "${out.stdout//typeset -C/}" == ${expected_output_pattern}	]] || err_exit "${testname}: Expected stdout to match ${ printf '%q\n' "${expected_output_pattern}" ; }, got ${ printf '%q\n' "${out.stdout}" ; }"
		[[ "${out.stderr}" == ''				]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
		(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"
	done

	return 0
}

# test whether poll(1) works with multiple fifos
# the test sends a single-character (single-byte or multi-byte)
# "token" around which loops between fifos for a given number
# of cycles
function test_fifo_circus1
{
	compound out=( typeset stdout stderr ; integer res )
	integer i
	typeset testname
cat >'poll_circus.sh' <<EOF
typeset -T circus_t=(
	typeset fifo_name
	integer in_fd
	integer out_fd

	function fifo_init
	{
		integer pid
		# fixme: we can't assign fd directly because ast-ksh.2012-08-13 has a bug
		# with "redirect"
		integer fd

		_.fifo_name="\$1"
		rm -f "\${_.fifo_name}"
		mkfifo "\${_.fifo_name}"

		# little "trick" with fifos - "redirect" would block if
		# there is no reader... so we create one in a
		# sub-process and kill it below after we opened the fd
		{ redirect {dummy}<"\${_.fifo_name}" ; for (( ;; )) ; do sleep 15 ; done ; } &
		(( pid=\$! ))

		redirect {fd}>"\${_.fifo_name}"
		(( _.out_fd=fd ))
		#printf '%s: outfd=%d\n' "\${_.fifo_name}" _.out_fd

		kill \$pid ; wait \$pid 2>'/dev/null'

		redirect {fd}<"\${_.fifo_name}"
		(( _.in_fd=fd ))
		#printf '%s: infd=%d\n' "\${_.fifo_name}" _.in_fd

		return 0
	}

	function fifo_cleanup
	{
		rm -f "\${_.fifo_name}"

		return 0
	}
)

function main
{
	typeset name subname
	typeset -M toupper up
	integer i
	integer -r num_cycles=18
	
	circus_t -A ar
	
	for name in 'a' 'b' 'c' 'd' ; do
		ar[\${name}].fifo_init "fifo_\${name}"
	done
	
	compound -A pollfd
	for name in "\${!ar[@]}" ; do
		subname="fifo_\${name}_in"
		pollfd[\${subname}]=( fd=\${ar[\${name}].in_fd}  events=( pollin='true'  pollerr='false' pollhup='false' pollnval='false' ) revents=() )
		subname="fifo_\${name}_out"
		pollfd[\${subname}]=( fd=\${ar[\${name}].out_fd} events=( pollout='true' pollerr='false' pollhup='false' pollnval='false' ) revents=() )
	done

	# main event loop	
	for (( i=0 ; i < num_cycles ; i++ )) ; do
		poll -t2 pollfd || print -u2 -f "poll failed, retval=%d'n" "\$?"
	
		# print results table
		printf '|'
		for name in "\${!ar[@]}" ; do
			up="\$name"
	
			subname="fifo_\${name}_in"
			if \${pollfd[\${subname}].revents.pollin}    ; then printf '%sI+' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollhup}   ; then printf '%sIH' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollerr}   ; then printf '%sIE' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollnval}  ; then printf '%sIN' "\${up}" ; else printf '...' ; fi
			subname="fifo_\${name}_out"
			if \${pollfd[\${subname}].revents.pollout}   ; then printf '%sO+' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollhup}   ; then printf '%sOH' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollerr}   ; then printf '%sOE' "\${up}" ; else printf '...' ; fi
			if \${pollfd[\${subname}].revents.pollnval}  ; then printf '%sON' "\${up}" ; else printf '...' ; fi
			printf '|'
		done
		printf '\n'

		# pass token around
		if \${pollfd[fifo_a_in].revents.pollin} && \${pollfd[fifo_b_out].revents.pollout}; then
			if read -u\${pollfd[fifo_a_in].fd} -N1 x ; then
				print -u\${pollfd[fifo_b_out].fd} -f "%s" "\$x" 2>'/dev/null'
			fi
		fi
		if \${pollfd[fifo_b_in].revents.pollin} && \${pollfd[fifo_c_out].revents.pollout}; then
			if read -u\${pollfd[fifo_b_in].fd} -N1 x ; then
				print -u\${pollfd[fifo_c_out].fd} -f "%s" "\$x" 2>'/dev/null'
			fi
		fi
		if \${pollfd[fifo_c_in].revents.pollin} && \${pollfd[fifo_d_out].revents.pollout}; then
			if read -u\${pollfd[fifo_c_in].fd} -N1 x ; then
				print -u\${pollfd[fifo_d_out].fd} -f "%s" "\$x" 2>'/dev/null'
			fi
		fi
		if \${pollfd[fifo_d_in].revents.pollin} && \${pollfd[fifo_a_out].revents.pollout}; then
			if read -u\${pollfd[fifo_d_in].fd} -N1 x ; then
				print -u\${pollfd[fifo_a_out].fd} -f "%s" "\$x" 2>'/dev/null'
			fi
		fi

		# send HUP around
		# we explicitly do NOT close the file descriptors here
		# to force poll(1) to set revents.pollnval=true
		if \${pollfd[fifo_a_in].revents.pollhup} ; then
			redirect {pollfd[fifo_a_in].fd}<&-;
			redirect {pollfd[fifo_b_out].fd}<&-;
		fi
		if \${pollfd[fifo_b_in].revents.pollhup} ; then
			redirect {pollfd[fifo_b_in].fd}<&-;
			redirect {pollfd[fifo_c_out].fd}<&-;
		fi
		if \${pollfd[fifo_c_in].revents.pollhup} ; then
			redirect {pollfd[fifo_c_in].fd}<&-;
			redirect {pollfd[fifo_d_out].fd}<&-;
		fi
		if \${pollfd[fifo_d_in].revents.pollhup} ; then
			redirect {pollfd[fifo_d_in].fd}<&-;
			redirect {pollfd[fifo_a_out].fd}<&-;
		fi

		# remove fds which have been reported as invalid via
		# revents.pollnval=true
		#
		# Notes:
		# - we do this to see whether poll(1) ignores such
		#   entries
		# - real applications may just use something like
		# \$ unset pollfd[fifo_a_in] # to remove the whole
		#   array node
		\${pollfd[fifo_a_in].revents.pollnval}  && (( pollfd[fifo_a_in].fd=-1  ))
		\${pollfd[fifo_a_out].revents.pollnval} && (( pollfd[fifo_a_out].fd=-1 ))
		\${pollfd[fifo_b_in].revents.pollnval}  && (( pollfd[fifo_b_in].fd=-1  ))
		\${pollfd[fifo_b_out].revents.pollnval} && (( pollfd[fifo_b_out].fd=-1 ))
		\${pollfd[fifo_c_in].revents.pollnval}  && (( pollfd[fifo_c_in].fd=-1  ))
		\${pollfd[fifo_c_out].revents.pollnval} && (( pollfd[fifo_c_out].fd=-1 ))
		\${pollfd[fifo_d_in].revents.pollnval}  && (( pollfd[fifo_d_in].fd=-1  ))
		\${pollfd[fifo_d_out].revents.pollnval} && (( pollfd[fifo_d_out].fd=-1 ))

		# send start token	
		if (( i==0 )) ; then
			# Use the Euro symbol (\u[20ac]) if the locale
			# uses a UTF-8 encoding
			typeset -r utf8_euro_char1=\$'\u[20ac]'
			typeset -r utf8_euro_char2=\$'\342\202\254'
			if (( (\${#utf8_euro_char1} == 1) && (\${#utf8_euro_char2} == 1) )) ; then
				print -u\${pollfd[fifo_a_out].fd} -f '\u[20ac]'
			else
				print -u\${pollfd[fifo_a_out].fd} -f 'X'
			fi
		fi

		# start sending hup around
		(( i==(num_cycles-7) )) && redirect {pollfd[fifo_a_out].fd}<&-;
	done

	for name in "\${!ar[@]}" ; do
		ar[\${name}].fifo_cleanup
	done

	print 'DONE'

	return 0
}

set -o nounset
set -o errexit

builtin mkfifo
builtin poll
builtin rm

main
exit $?
EOF

	typeset IFS=''
	typeset -r -a expected_output_pattern=(
		$'|............AO+.........|............BO+.........|............CO+.........|............DO+.........|\n'
		$'|AI+.........AO+.........|............BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|BI+.........BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|............BO+.........|CI+.........CO+.........|............DO+.........|\n'
		$'|............AO+.........|............BO+.........|............CO+.........|DI+.........DO+.........|\n'
		$'|AI+.........AO+.........|............BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|BI+.........BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|............BO+.........|CI+.........CO+.........|............DO+.........|\n'
		$'|............AO+.........|............BO+.........|............CO+.........|DI+.........DO+.........|\n'
		$'|AI+.........AO+.........|............BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|BI+.........BO+.........|............CO+.........|............DO+.........|\n'
		$'|............AO+.........|............BO+.........|CI+.........CO+.........|............DO+.........|\n'
		$'|AI+AIH...............AON|............BO+.........|............CO+.........|DI+.........DO+.........|\n'
		$'|.........AIN............|BI+BIH...............BON|............CO+.........|DI+.........DO+.........|\n'
		$'|........................|.........BIN............|CI+CIH...............CON|DI+.........DO+.........|\n'
		$'|........................|........................|.........CIN............|DI+DIH...............DON|\n'
		$'|........................|........................|........................|.........DIN............|\n'
		$'|........................|........................|........................|........................|\n'
		$'DONE'
	)

	testname="$0/plain"
	out.stderr="${ { out.stdout="${ ${SHELL} 'poll_circus.sh' ; (( out.res=$? )) ; }" ; } 2>&1 ; }"
	[[ "${out.stdout}" == "${expected_output_pattern[*]}"	]] || err_exit "${testname}: Expected stdout to match ${ printf '%q\n' "${expected_output_pattern[*]}" ; }, got ${ printf '%q\n' "${out.stdout}" ; }"
	[[ "${out.stderr}" == ''				]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
	(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"

	testname="$0/compiled"
	out.stderr="${ { out.stdout="${ ${SHCOMP} -n 'poll_circus.sh' 'poll_circus.shbin' && ${SHELL} 'poll_circus.shbin' ; (( out.res=$? )) ; }" ; } 2>&1 ; }"
	[[ "${out.stdout}" == "${expected_output_pattern[*]}"	]] || err_exit "${testname}: Expected stdout to match ${ printf '%q\n' "${expected_output_pattern[*]}" ; }, got ${ printf '%q\n' "${out.stdout}" ; }"
	[[ "${out.stderr}" == ''				]] || err_exit "${testname}: Expected empty stderr, got ${ printf '%q\n' "${out.stderr}" ; }"
	(( out.res == 0 )) || err_exit "${testname}: Unexpected exit code ${out.res}"

	rm -f 'poll_circus.sh' 'poll_circus.shbin'

	return 0
}

# run tests
builtin poll	|| { err_exit 'poll builtin not found.'; exit 1; }
builtin rmdir	|| { err_exit 'rmdir builtin not found.'; exit 1; }

test1
test_sparse_array1
test_fifo_circus1

# cleanup
cd "${ocwd}"
rmdir "${tmpdir}" || err_exit "Cannot remove temporary directory ${tmpdir}."


# tests done
exit $((Errors<125?Errors:125))
