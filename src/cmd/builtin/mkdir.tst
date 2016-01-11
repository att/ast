# : : generated from /home/gsf/src/cmd/builtin/mkdir.rt by mktest : : #

# regression tests for the mkdir command

UNIT mkdir

TEST 01 '"-m =" vs. umask'

	EXEC	-m '=rx' d
		INPUT -n -
		OUTPUT -
		ERROR -n -
		UMASK 000

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 007

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0550 (r-xr-x---)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 070

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0505 (r-x---r-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 077

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0500 (r-x------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 700

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0055 (---r-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 707

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0050 (---r-x---)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 770

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0005 (------r-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m '=rx' d
		UMASK 777

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m u=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0577 (r-xrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m g=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0757 (rwxr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m o=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0775 (rwxrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m ug=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0557 (r-xr-xrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m uo=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0575 (r-xrwxr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m go=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0755 (rwxr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m a=rx d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -777 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0000 (---------)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m 222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0222 (-w--w--w-)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m +222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0777 (rwxrwxrwx)'

	PROG	rmdir d
		OUTPUT -

	EXEC	-m -222 d

	PROG	chmod -v + d
		OUTPUT - 'd: mode changed to 0555 (r-xr-xr-x)'

	PROG	rmdir d
		OUTPUT -

TEST 02 '-p vs umask vs intermediate and final modes'

	EXEC	-pv a/b/c
		INPUT -n -
		OUTPUT -
		ERROR - $'a: directory created\na/b: directory created\na/b/c: directory created'
		UMASK 0202

	PROG	chmod -v + a a/b a/b/c
		OUTPUT - $'a: mode changed to 0775 (rwxrwxr-x)
a/b: mode changed to 0775 (rwxrwxr-x)
a/b/c: mode changed to 0575 (r-xrwxr-x)'
		ERROR -n -

	EXEC	-v d
		OUTPUT -
		ERROR - 'd: directory created'
