# : : generated from /home/gsf/src/cmd/builtin/chmod.rt by mktest : : #

# regression tests for the chmod command

UNIT chmod

TEST 01 '"=" vs. umask'

	PROG	touch f
		INPUT -n -
		OUTPUT -
		ERROR -n -

	EXEC	777 f
		UMASK 000

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 007

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 070

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 077

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,u=rw' f
		OUTPUT -

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 700

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 707

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 770

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	EXEC	777 f
		OUTPUT -
		UMASK 777

	EXEC	-c 777 f

	EXEC	-c '=' f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	EXEC	-c '=rw' f
		OUTPUT -

	EXEC	-c '=,u=rw' f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	EXEC	-c '=,g=rw' f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	EXEC	-c '=,o=rw' f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	EXEC	-c '=,ug=rw' f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	EXEC	-c '=,uo=rw' f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	EXEC	-c '=,go=rw' f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	EXEC	-c '=,a=rw' f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

TEST 02 'X vs. DIR and REG'

	PROG	touch f
		INPUT -n -
		OUTPUT -
		ERROR -n -
		UMASK 000

	EXEC	444 f

	EXEC	-c u=rX f

	EXEC	-c o+x f
		OUTPUT - 'f: mode changed to 0445 (r--r--r-x)'

	EXEC	-c u=rX f
		OUTPUT - 'f: mode changed to 0545 (r-xr--r-x)'

	PROG	mkdir d
		OUTPUT -

	EXEC	400 d

	EXEC	-c u=rX d
		OUTPUT - 'd: mode changed to 0500 (r-x------)'

	EXEC	-c o=rX d
		OUTPUT - 'd: mode changed to 0505 (r-x---r-x)'
