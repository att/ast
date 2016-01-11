# : : generated from /home/gsf/src/cmd/builtin/mkfifo.rt by mktest : : #

# regression tests for the mkfifo command

UNIT mkfifo

TWD

TEST 01 '"-m =" vs. umask'

	EXEC	-m '=rw' f
		INPUT -n -
		OUTPUT -
		ERROR -n -
		UMASK 000

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 007

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0660 (rw-rw----)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 070

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0606 (rw----rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 077

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0600 (rw-------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 700

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0066 (---rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 707

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0060 (---rw----)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 770

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0006 (------rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m '=rw' f
		UMASK 777

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m ug=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m uo=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m go=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m a=rw f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -777 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0000 (---------)'

	PROG	rm f
		OUTPUT -

	EXEC	-m 222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0222 (-w--w--w-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -222 f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0444 (r--r--r--)'

	PROG	rm f
		OUTPUT -

TEST 02 VSC

	EXEC	-m +rw f
		INPUT -n -
		OUTPUT -
		ERROR -n -
		UMASK 027

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0776 (rwxrwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0776 (rwxrwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0466 (r--rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +rw f
		UMASK u=rx,g=rx,o=rwx

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0664 (rw-rw-r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +rw f
		UMASK u=rwx,g=rx,o=rx

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0466 (r--rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +rw f
		UMASK u=rwx,g=rx,o=x

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0466 (r--rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +rw f
		UMASK u=rwx,g=rx,o=r

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0776 (rwxrwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0776 (rwxrwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0466 (r--rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +rw f
		UMASK u=rwx,g=rx,o=rwx

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +x f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m u=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0766 (rwxrw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m g=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0676 (rw-rwxrw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o=rwx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m o+wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0667 (rw-rw-rwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m +wx f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0777 (rwxrwxrwx)'

	PROG	rm f
		OUTPUT -

	EXEC	-m + f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -

	EXEC	-m -w f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0464 (r--rw-r--)'

	PROG	rm f
		OUTPUT -

	EXEC	-m - f

	PROG	chmod -v + f
		OUTPUT - 'f: mode changed to 0666 (rw-rw-rw-)'

	PROG	rm f
		OUTPUT -
