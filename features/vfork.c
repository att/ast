// This is for the Meson config stage to check if vfork() acts like
// ksh expects.  We need vfork() to pause the parent and share memory
// with the child until the child execs or exits.
//
// These systems do pause the parent and share memory:
// - FreeBSD: https://www.freebsd.org/cgi/man.cgi?vfork
// - Illumos: https://illumos.org/man/vfork
// - Linux: http://man7.org/linux/man-pages/man2/vfork.2.html
// - NetBSD: http://netbsd.gw.com/cgi-bin/man-cgi?vfork
//
// This system doesn't share memory:
// - OpenBSD: https://man.openbsd.org/vfork

#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    pid_t pid;
    int m;
    volatile int exec_errno;
    volatile int *volatile exec_errno_ptr;

    // Mimic how libast/misc/spawnvex.c calls vfork().
    exec_errno = 0;
    exec_errno_ptr = &exec_errno;
    pid = vfork();
    if (pid == -1) {
        return 1;  // fail, can't fork
    } else if (!pid) {
        // As child, pretend to exec something.
        *exec_errno_ptr = ENOEXEC;
        _exit(126);
    }

    // As parent, read errno from child, immediately after returning
    // from vfork() and before calling waitpid().
    m = *exec_errno_ptr;
    while (waitpid(pid, NULL, 0) == -1 && errno == EINTR) continue;
    if (m == ENOEXEC) {
        return 0;  // pass
    } else {
        return 1;  // fail, got wrong errno
    }
}
