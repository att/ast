libast_files += [
    'misc/astintercept.c', 'misc/cmdarg.c', 'misc/conformance.c',
    'misc/debug.c', 'misc/error.c', 'misc/errorf.c', 'misc/errormsg.c',
    'misc/errorx.c', 'misc/fastfind.c', 'misc/fgetcwd.c', 'misc/fmtrec.c',
    'misc/fs3d.c',
    'misc/getenv.c', 'misc/glob.c',
    'misc/mime.c', 'misc/optctx.c', 'misc/optesc.c',
    'misc/optget.c', 'misc/optjoin.c', 'misc/procclose.c', 'misc/procfree.c',
    'misc/procopen.c', 'misc/procrun.c', 'misc/recfmt.c', 'misc/reclen.c',
    'misc/recstr.c', 'misc/setenviron.c', 'misc/sigcrit.c', 'misc/sigdata.c',
    'misc/signal.c', 'misc/spawnvex.c', 'misc/stack.c', 'misc/state.c',
    'misc/stk.c', 'misc/systrace.c', 'misc/translate.c', 'misc/univdata.c'
]


_lib_posix_spawn_test = '''
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/wait.h>
	#include <spawn.h>
	#include <signal.h>
	#include <fcntl.h>
	#include <string.h>
	#include <stdio.h>

	/* if it uses fork() why bother? */
	#undef fork
	pid_t fork (void) { printf("uses fork()"); return -1; }
	pid_t _fork (void) { printf("uses _fork()"); return -1; }
	pid_t __fork (void) { printf("uses __fork()"); return -1; }

	int main(int argc, char **argv) {
        #if __CYGWIN__
            // Not only does it not work it causes the meson configure step to
            // take tens or minutes to complete and results in a huge cascade
            // of child processes.
            printf("Cygwin doesn't have a working posix_spawn()\n");
            _exit(0);
        #else  // __CYGWIN__
		char*			s;
		pid_t			pid;
		posix_spawnattr_t	attr;
		int			n;
		int			status;
		char*			cmd[4];
		char			tmp[1024];
		if (argv[2])
			_exit(signal(SIGHUP, SIG_DFL) != SIG_IGN);
		signal(SIGHUP, SIG_IGN);
		if (posix_spawnattr_init(&attr))
		{
			printf("posix_spawnattr_init() FAILED\n");
			_exit(0);
		}
		if (posix_spawnattr_setpgroup(&attr, 0))
		{
			printf("posix_spawnattr_setpgroup() FAILED\n");
			_exit(0);
		}
		if (posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP))
		{
			printf("posix_spawnattr_setflags() FAILED\n");
			_exit(0);
		}
		/* first try an a.out and verify that SIGHUP is ignored */
		cmd[0] = argv[0];
		cmd[1] = argv[1];
		cmd[2] = "test";
		cmd[3] = 0;
		if (posix_spawn(&pid, cmd[0], 0, &attr, cmd, 0))
		{
			printf("posix_spawn() FAILED\n");
			_exit(0);
		}
		status = 1;
		if (wait(&status) < 0)
		{
			printf("wait() FAILED\n");
			_exit(0);
		}
		if (status != 0)
		{
			printf("SIGHUP ignored in parent not ignored in child\n");
			_exit(0);
		}
		/* must return exec-type errors or its useless to us *unless* there is no [v]fork() */
		n = strlen(cmd[0]);
		if (n >= (sizeof(tmp) - 3))
		{
			printf("test executable path too long\n");
			_exit(0);
		}
		strcpy(tmp, cmd[0]);
		tmp[n] = '.';
		tmp[n+1] = 's';
		tmp[n+2] = 'h';
		tmp[n+3] = 0;
		if ((n = open(tmp, O_CREAT|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO)) < 0 ||
                    chmod(tmp, S_IRWXU|S_IRWXG|S_IRWXO) < 0 ||
                    write(n, "exit 99\n", 8) != 8 ||
                    close(n) < 0)
		{
			printf("test script create FAILED\n");
			_exit(0);
		}
		cmd[0] = tmp;
		n = 0; /* 0 means reject */
		pid = -1;
		if (posix_spawn(&pid, cmd[0], 0, &attr, cmd, 0))
		{
			n = 2;
			printf("ENOEXEC produces posix_spawn() error (BEST)\n");
		}
		else if (pid == -1)
			printf("ENOEXEC returns pid == -1\n");
		else if (wait(&status) != pid)
			printf("ENOEXEC produces no child process\n");
		else if (!WIFEXITED(status))
			printf("ENOEXEC produces signal exit\n");
		else
		{
			status = WEXITSTATUS(status);
			if (status == 127)
			{
				n = 1;
				printf("ENOEXEC produces exit status 127 (GOOD)\n");
			}
			else if (status == 99)
				printf("ENOEXEC invokes sh\n");
			else if (status == 0)
				printf("ENOEXEC reports no error\n");
			else
				printf("ENOEXEC produces non-zero exit status\n");
		}
		_exit(n);
        #endif  // __CYGWIN__
        }
'''

# This will output the test has failed, but that's how it was written
_lib_posix_spawn_result = cc.run(
    _lib_posix_spawn_test, name: 'posix_spawn exists and it works and its worth using ')

libast_c_args += [ '-D_lib_posix_spawn=@0@'.format(_lib_posix_spawn_result.returncode()) ]
