#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

static char fname_hlink[PATH_MAX] = "";
static char fname_slink[PATH_MAX] = "";

#define EXPECTED_SYMLINK_MODE 0751

static void cleanup() {
    if (*fname_hlink) unlink(fname_hlink);
    if (*fname_slink) unlink(fname_slink);
}

int main() {
    struct stat statbuf;
    int rv;
    int pid = getpid();

    // Create a file.
    snprintf(fname_hlink, PATH_MAX, "lchmod_fallback_hlink.%d", pid);
    rv = open(fname_hlink, O_WRONLY | O_CREAT, 0);
    if (rv == -1) {
        fprintf(stderr, "<E> open rv %d  errno %d\n", rv, errno);
        goto err_exit;
    }
    close(rv);

    // Symlink to the file.
    snprintf(fname_slink, PATH_MAX, "lchmod_fallback_slink.%d", pid);
    rv = symlink(fname_hlink, fname_slink);
    if (rv == -1) {
        fprintf(stderr, "<E> symlink rv %d  errno %d\n", rv, errno);
        goto err_exit;
    }

    // Change the modes of the symlink -- not the file.
    rv = fchmodat(AT_FDCWD, fname_slink, EXPECTED_SYMLINK_MODE, AT_SYMLINK_NOFOLLOW);
    if (rv == -1) {
        fprintf(stderr, "<E> fchmodat rv %d  errno %d\n", rv, errno);
        goto err_exit;
    }

    rv = lstat(fname_hlink, &statbuf);
    if (rv == -1) {
        fprintf(stderr, "<E> lstat(hlink %s)  rv %d  errno %d\n", fname_hlink, rv, errno);
        goto err_exit;
    }
    fprintf(stdout, "<I> lstat(hlink %s)  mode 0%o\n", fname_hlink, statbuf.st_mode);
    if ((statbuf.st_mode & 0777) != 0) {
        fprintf(stderr, "<E> lstat(hlink %s)  mode should be 0%o is 0%o\n", fname_hlink, 0,
                statbuf.st_mode);
        goto err_exit;
    }

    rv = lstat(fname_slink, &statbuf);
    if (rv == -1) {
        fprintf(stderr, "<E> lstat(slink %s)  rv %d  errno %d\n", fname_slink, rv, errno);
        goto err_exit;
    }
    fprintf(stdout, "<I> lstat(slink %s)  mode 0%o\n", fname_slink, statbuf.st_mode);
    if ((statbuf.st_mode & 0777) != EXPECTED_SYMLINK_MODE) {
        fprintf(stderr, "<E> lstat(slink %s)  mode should be 0%o is 0%o\n", fname_slink,
                EXPECTED_SYMLINK_MODE, statbuf.st_mode);
        goto err_exit;
    }

    cleanup();
    return 0;

err_exit:
    cleanup();
    return 1;
}
